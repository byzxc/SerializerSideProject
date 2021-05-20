#ifndef _SERIALIZER_HPP_
#define _SERIALIZER_HPP_

#include <array>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>

#include "ContainerChecker.hpp"
#include "TypeTraits.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rttr/registration.h"
#include "rttr/type.h"

#ifdef JSON
#define JSON_SERIALIZE(filePath, RTTRInstance) ((void)0)

#else

#define JSON_SERIALIZE(filePath, RTTRInstance) \
 { \
  JSON::SerializeToFile(filePath, RTTRInstance); \
 }

#endif

namespace JSON
{
    using namespace rapidjson;
    using namespace rttr;

    class Writer : private PrettyWriter<StringBuffer>
    {
    public:
        // Default Constructor
        Writer() = default;

        // Parametrized Constructor
        Writer(PrettyWriter<StringBuffer>& writer) : m_Writer{ &writer }
        {
        }

        void StartObject() const
        {
            m_Writer->StartObject();
        }

        void EndObject() const
        {
            m_Writer->EndObject();
        }

        void StartArray() const
        {
            m_Writer->StartArray();
        }

        void EndArray() const
        {
            m_Writer->EndArray();
        }

        // PrettyWriter Option or Single Line Array
        void SetFormatOptions(PrettyFormatOptions options) const
        {
            m_Writer->SetFormatOptions(options);
        }

        void SetMaxDecimalPlace(int maxDecimalPlaces) const
        {
            m_Writer->SetMaxDecimalPlaces(maxDecimalPlaces);
        }

        void PutKey(const std::string& keyName) const
        {
            m_Writer->Key(keyName.c_str());
        }

        void PutNull() const
        {
            m_Writer->Null();
        }

        template <typename Type>
        void PutValue(const Type& key)
        {
            if constexpr (TYPETRAITS::are_same<Type, uint64_t >::value)
            {
                m_Writer->Uint64(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, int64_t >::value)
            {
                m_Writer->Int64(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, double>::value || TYPETRAITS::are_same<Type, float>::value)
            {
                m_Writer->Double(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, bool>::value)
            {
                m_Writer->Bool(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, int >::value)
            {
                m_Writer->Int(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, unsigned >::value)
            {
                m_Writer->Uint(key);
            }
            else if constexpr (TYPETRAITS::are_same<Type, std::string>::value || TYPETRAITS::are_same<Type, const char*>::value)
            {
                m_Writer->String(key.c_str());
            }
            else
            {
                // Should not hit this
                std::cout << "Unable to inspect the defined Type, Seek Fail" << std::endl;
                // Put null if seeks fail
                m_Writer->Null();
                // Probably do an assert here in the future.
            }
        }

        // Specialized for string if user did not pass in a std::string but passed in "..."
        template <>
        void PutValue<std::string>(const std::string& key)
        {
            m_Writer->String(key.c_str());
        }

        // For Sequence Containers -> (array, vector, deque, list, forward_list)
        // For Associative Containers -> (map, unordered_map)
        // Containee = std::allocator<Type>
        template<typename Type, template<typename, typename...> class Container, typename... Containee>
        void PutContainerValue(const std::string& key, const Container<Type, Containee...>& valueContainer)
        {
            PutKey(key);
            PutContainerValue(valueContainer);
        }

        // *********************************************************
        // *Using RTTR Library API to help check the type of the variable before writing it to JSON version
        // *********************************************************
        bool WriteAtomicTypes(const type& type, const variant& variant)
        {
            // Basic Primitive & Floating Point Types like int, unsigned, std::string, float, double
            if (type.is_arithmetic())
            {
                if (type == type::get<bool>() || type == type::get<char>())
                {
                    this->PutValue(variant.to_bool());
                }
                else if (type == type::get<int8_t>())
                {
                    this->PutValue(variant.to_int8());
                }
                else if (type == type::get<int16_t>())
                {
                    this->PutValue(variant.to_int16());
                }
                else if (type == type::get<int32_t>())
                {
                    this->PutValue(variant.to_int32());
                }
                else if (type == type::get<int64_t>())
                {
                    this->PutValue(variant.to_int64());
                }
                else if (type == type::get<uint8_t>())
                {
                    this->PutValue(variant.to_uint8());
                }
                else if (type == type::get<uint16_t>())
                {
                    this->PutValue(variant.to_uint16());
                }
                else if (type == type::get<uint32_t>())
                {
                    this->PutValue(variant.to_uint32());
                }
                else if (type == type::get<uint64_t>())
                {
                    this->PutValue(variant.to_uint64());
                }
                else if (type == type::get<float>() || type == type::get<double>())
                {
                    this->PutValue(variant.to_double());
                }
                return true;
            }

            // Specialized version for std::string
            if (type == type::get<std::string>() || type == type::get<const char*>())
            {
                this->PutValue<std::string>(variant.to_string());
                return true;
            }

            // Check if enum
            if (type.is_enumeration())
            {
                bool canConvertToString = false;
                // Needs to check if its possible to convert custom types(variant) to std::string
                const std::string result = variant.to_string(&canConvertToString);

                if (canConvertToString)
                {
                    this->PutValue<std::string>(variant.to_string());
                }
                else
                {
                    canConvertToString = false;
                    const uint64_t value = variant.to_uint64(&canConvertToString);
                    if (canConvertToString)
                    {
                        this->PutValue(value);
                    }
                    else
                    {
                        this->PutNull();
                    }
                }
                return true;
            }

            // If all fails, then return false, probably will have to do a assert here
            // assert("Not Primitive, Floating, std::string, Enum types");
            return false;
        }

        void WriteArray(const variant_sequential_view& variantView)
        {
            this->StartArray();
            // variantView can store containers/arithmetic/std::string/enums inside
            // Those are actually copied over to variant, for e.g. I can std::vector<int> cat{1,2,3,4,5}
            // And then i variant var = cat;
            // Then variantView = var.create_variant_sequential_view();
            // So this is how variantView came about, inside it will store a std::vector<int>.
            for (const variant& item : variantView)
            {
                // Check if the item inside is a sequential container
                if (item.is_sequential_container())
                {
                    // Recursively call this function to add the item in one by one using WriteAtomicTypes
                    WriteArray(item.create_sequential_view());
                }
                else
                {
                    // A wrapper type is a class which encapsulate an instance of another type. This encapsulate type is also called wrapped type.
                    variant wrappedVariant = item.extract_wrapped_value();
                    type valueType = wrappedVariant.get_type();

                    if (valueType.is_arithmetic() || valueType == type::get<std::string>() || valueType.is_enumeration())
                    {
                        WriteAtomicTypes(valueType, wrappedVariant);
                    }
                    else
                    {
                        // Is an object, object refers to your class/struct object
                        WriteToJSONRecursively(wrappedVariant);
                    }
                }
            }
            this->EndArray();
        }

        void WriteAssociativeContainer(const variant_associative_view& variantView)
        {
            static const string_view key_name("key");
            static const string_view value_name("value");

            this->StartArray();

            if (variantView.is_key_only_type())
            {
                for (const std::pair<variant, variant>& item : variantView)
                {
                    WriteVariant(item.first);
                }
            }
            else
            {
                for (const std::pair<variant, variant>& item : variantView)
                {
                    this->StartObject();

                    this->PutKey(key_name.data());
                    WriteVariant(item.first);

                    this->PutKey(value_name.data());
                    WriteVariant(item.second);

                    this->EndObject();
                }
            }
            this->EndArray();
        }

        bool WriteVariant(const variant& variant)
        {
            type valueType = variant.get_type();
            type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;
            bool isWrappedType = wrappedType != valueType;

            if (WriteAtomicTypes(isWrappedType ? wrappedType : valueType, isWrappedType ? variant.extract_wrapped_value() : variant))
            {
                // Do nothing
            }
            else if (variant.is_sequential_container())
            {
                WriteArray(variant.create_sequential_view());
            }
            else if (variant.is_associative_container())
            {
                WriteAssociativeContainer(variant.create_associative_view());
            }
            else
            {
                const auto childProperties = isWrappedType ? wrappedType.get_properties() : valueType.get_properties();
                if (!childProperties.empty())
                {
                    WriteToJSONRecursively(variant);
                }
                else
                {
                    bool canConvertToString = false;
                    const std::string value = variant.to_string(&canConvertToString);
                    if (!canConvertToString)
                    {
                        this->PutKey(value);
                        return false;
                    }
                    this->PutKey(value);
                }
            }
            return true;
        }

        void WriteToJSONRecursively(const instance& rttrObject)
        {
            this->StartObject();

            instance obj = rttrObject.get_type().get_raw_type().is_wrapper() ? rttrObject.get_wrapped_instance() : rttrObject;

            auto propertiesList = obj.get_derived_type().get_properties();
            for (property propertie : propertiesList)
            {
                // Skip properties that are marked NO_SERIALIZE
                if (propertie.get_metadata("NO_SERIALIZE"))
                {
                    continue;
                }

                variant propertyValue = propertie.get_value(obj);

                if (!propertyValue)
                {
                    // Cannot serialize, because we cannot retrieve the value
                    std::cerr << "Unable to retrieve property value!" << std::endl;
                    continue;
                }

                const string_view name = propertie.get_name();
                this->PutKey(name.data());

                if (!WriteVariant(propertyValue))
                {
                    std::cerr << "Cannot serialize property: " << name << std::endl;
                }
            }

            this->EndObject();
        }

        // Getters
        PrettyWriter<StringBuffer>* GetPrettyWriter() const
        {
            return m_Writer;
        }

        template<typename... Containee>
        void PutContainerTuple(const std::string& key, const std::tuple<Containee...>& valueContainer)
        {
            PutKey(key);
            PutContainerTuple(valueContainer);
        }

        // Private Variables
    private:
        PrettyWriter<StringBuffer>* m_Writer = nullptr;

        // Private Functions
    private:
        //TODO:: Multimap , Multiset not fully tested
        template<typename Type, template<typename, typename...> class Container, typename... Containee>
        void PutContainerValue(const Container<Type, Containee...>& valueContainer)
        {
            StartArray();
            // When dealing with containers, I would want the JSON file to be in one line
            SetFormatOptions(kFormatSingleLineArray);

            if constexpr (TYPETRAITS::is_pair<std::decay_t<decltype(*valueContainer.begin())>>::value)
            {
                for (const auto& itr : valueContainer)
                {
                    // Safety Check for same Type, unable to StartObject for same type
                    if (TYPETRAITS::are_same<Type, Containee...>::value)
                    {
                        PutKey(itr.first);
                        PutValue(itr.second);
                    }
                    else
                    {
                        StartObject();
                        PutKey(itr.first);
                        PutValue(itr.second);
                        EndObject();
                    }
                }
            }
            else
            {
                for (const auto& itr : valueContainer)
                {
                    PutValue(itr);
                }
            }
            EndArray();
        }

        template<typename Function, typename Tuple>
        void GetAllTupleElements(Function&& function, Tuple&& tuple)
        {
            std::apply([&function](auto&&... Args)
                {
                    (function(std::forward<decltype(Args)>(Args)), ...);
                },
                std::forward<Tuple>(tuple));
        }

        template<typename... Containee>
        void PutContainerTuple(const std::tuple<Containee...>& valueContainer)
        {
            StartArray();
            // When dealing with containers, I would want the JSON file to be in one line
            SetFormatOptions(kFormatSingleLineArray);

            GetAllTupleElements([&](auto&& Args)
                {
                    PutValue(Args);
                }, valueContainer);

            EndArray();
        }
    };

    class Reader
    {
    public:
        // Default Constructor
        Reader() = default;

        // Parametrized Constructor
        Reader(Value& data) : m_Data{ &data }
        {
        }

        bool IsNull() const
        {
            return m_Data->IsNull();
        }

        bool IsObject() const
        {
            return m_Data->IsObject();
        }

        bool IsArray() const
        {
            return m_Data->IsArray();
        }

        bool HasMember(const std::string& name) const
        {
            return m_Data->HasMember(name.c_str());
        }

        // More optimized version compared to HasMember where it perform 1 seek instead of 2 seeks
        GenericValue<UTF8<>>::MemberIterator FindMember(const std::string& name) const
        {
            // Checking through the array if this member exist,
            // Return false if it reaches the end with no result
            return m_Data->FindMember(name.c_str());
        }

        Value& GetRawValue(const std::string& Key) const
        {
            return (*m_Data)[Key.c_str()];
        }

        template <typename TValue>
        TValue GetValue(const std::string& Key)
        {
            // Check if the key given is valid
            if (m_Data->FindMember(Key.c_str()) == m_Data->MemberEnd())
            {
                std::cout << "Key provided is not valid" << std::endl;
                //TODO:: Replace with own assert
                assert(false);
            }

            const Value& value = (*m_Data)[Key.c_str()];

            if constexpr (TYPETRAITS::are_same<TValue, uint64_t>::value)
            {
                return value.GetUint64();
            }

            if constexpr (TYPETRAITS::are_same<TValue, int64_t>::value)
            {
                return value.GetInt64();
            }

            if constexpr (TYPETRAITS::are_same<TValue, double>::value)
            {
                return value.GetDouble();
            }

            if constexpr (TYPETRAITS::are_same<TValue, float>::value)
            {
                return value.GetFloat();
            }

            if constexpr (TYPETRAITS::are_same<TValue, bool>::value)
            {
                return value.GetBool();
            }

            if constexpr (TYPETRAITS::are_same<TValue, int>::value)
            {
                return value.GetInt();
            }

            if constexpr (TYPETRAITS::are_same<TValue, unsigned>::value)
            {
                return value.GetUint();
            }

            if constexpr (TYPETRAITS::are_same<TValue, std::string>::value || TYPETRAITS::are_same<TValue, const char*>::value)
            {
                return value.GetString();
            }
        }

        template <typename T, typename... TValue, typename Encoding, typename Allocator = RAPIDJSON_DEFAULT_ALLOCATOR>
        T GetValue(const GenericValue<Encoding, Allocator>& Value)
        {
            if constexpr (TYPETRAITS::are_same<T, uint64_t>::value)
            {
                return Value.GetUint64();
            }

            if constexpr (TYPETRAITS::are_same<T, int64_t>::value)
            {
                return Value.GetInt64();
            }

            if constexpr (TYPETRAITS::are_same<T, double>::value)
            {
                return Value.GetDouble();
            }

            if constexpr (TYPETRAITS::are_same<T, float>::value)
            {
                return Value.GetFloat();
            }

            if constexpr (TYPETRAITS::are_same<T, bool>::value)
            {
                return Value.GetBool();
            }

            if constexpr (TYPETRAITS::are_same<T, int>::value)
            {
                return Value.GetInt();
            }

            if constexpr (TYPETRAITS::are_same<T, unsigned>::value)
            {
                return Value.GetUint();
            }

            if constexpr (TYPETRAITS::are_same<T, std::string>::value || TYPETRAITS::are_same<T, const char*>::value)
            {
                return Value.GetString();
            }
        }

        template<typename Type, template<typename, typename...> class Container, typename... Containee>
        void GetContainerValue(Container<Type, Containee...>& valueContainer, const std::string& Key)
        {
            // Check if the key given is valid
            if (m_Data->FindMember(Key.c_str()) == m_Data->MemberEnd())
            {
                std::cout << "Key provided is not valid" << std::endl;
                //TODO:: Replace with own assert
                assert(false);
            }

            const Value& data = (*m_Data)[Key.c_str()];

            // Check if container contains pair -> map, multimap, unordered map...
            if constexpr (TYPETRAITS::is_pair<std::decay_t<decltype(*valueContainer.begin())>>::value)
            {
                // Check if m_Data is null
                if (IsNull())
                {
                    std::cout << "m_Data is null" << std::endl;
                    assert(false);
                    // Might want to do some assert here
                }

                if (data.IsArray())
                {
                    for (const auto& itr : data.GetArray())
                    {
                        if (itr.IsObject())
                        {
                            for (auto m = itr.MemberBegin(); m != itr.MemberEnd(); ++m)
                            {
                                const std::string key = m->name.GetString();
                                auto value = GetValue<Containee...>(m->value);
                                valueContainer.emplace(std::make_pair(key, value));
                            }
                        }
                    }
                }
            }
            // This is vector, deque, set, list...
            else
            {
                if (data.IsArray())
                {
                    for (SizeType i = 0; i < data.Size(); ++i)
                    {
                        valueContainer.push_back(GetValue<Type>(data[i]));
                    }
                }
            }
        }

        // TODO:: WIP
        template<typename Type, template<typename, typename...> class Container, typename... Containee>
        void GetTupleValue(Container<Type, Containee...>& valueContainer, const std::string& Key)
        {
            //const Value& data = (*m_Data)[Key.c_str()];

            //if (data.IsArray())
            //{
            //    for (SizeType i = 0; i < data.Size(); ++i)
            //    {
            //        std::cout << GetValue<Containee...>(data[i]) << std::endl;
            //    }

            //    //auto tupleSize = std::tuple_size<decltype(valueContainer)>::value;
            //    //std::cout << tupleSize << std::endl;
            //}
        }

    private:
        Value* m_Data = nullptr;
        instance rttrObject{};
    };

    // This class is for whoever wish to have their data to be serialized. Some examples are components who will inherit this class
    // Only Inherit this if you do not want to use RTTR, **which you should be using**
    class SerializeBase
    {
    public:
        virtual ~SerializeBase() = default;
        virtual void Deserialize(Reader reader) = 0;
        virtual void Serialize(Writer writer) = 0;

    private:
    };

    // This class is for the owner that is responsible for calling SerializeToFile or DeserializeFromFile function
    class Serialization : public SerializeBase
    {
    public:
        // Delete Copy Constructor & Assignment Operator, Idw this class to be copyable
        Serialization(const Serialization&) = delete;
        Serialization& operator=(const Serialization&) = delete;

        // Default Constructor
        Serialization() = default;

        void SerializeToFile(const std::filesystem::path& filePath)
        {
            std::ofstream file{ filePath };
            // Check if file is in good bit
            if (file.good())
            {
                // Retrieve the strings from SerializeFunction
                const std::string stringBuffer = Serialize();
                file << stringBuffer;
            }
            else
            {
                // Do some Assert to show SerializeToFile failed
            }
        }

        // Default Serialize Function
        void Serialize(Writer writer) override
        {
            // Testing //
            //std::string dog{ "dog" };
            //variant variantType = dog;
            //writer.WriteVariant(variantType);
            //// This should be overriden by the class that inherits Serialize //
            //std::vector<std::string> dogVector;
            //dogVector.push_back("Cb");
            //dogVector.push_back("Knn");
            //writer.PutContainerValue("DogVector", dogVector);

            //std::map<std::string, int> cowMap;
            //cowMap.emplace("cow", 1);
            //cowMap.emplace("cownnie", 2);
            //writer.PutContainerValue("CowMap", cowMap);

            //writer.PutKey("alllah");
            //writer.PutValue(1);
        }

        // rttrObj is the object where all your properties and methods are stored
        virtual std::string Serialize()
        {
            StringBuffer buffer;
            PrettyWriter<StringBuffer> writer(buffer);
            const Writer ownWriter{ writer };
            ownWriter.StartObject();
            Serialize(writer);
            ownWriter.EndObject();
            return buffer.GetString();
        }

        void DeserializeFromFile(const std::filesystem::path& filePath)
        {
            std::ifstream file{ filePath };

            // Check if the file is good first
            if (file.good())
            {
                std::stringstream buffer;
                buffer << file.rdbuf();
                Deserialize(buffer.str());
            }
        }

        bool InitDocument(const std::string& validJSONName, rapidjson::Document& doc)
        {
            if (validJSONName.empty())
            {
                return false;
            }

            //Return false if parsing of document has error
            return !doc.Parse(validJSONName.c_str()).HasParseError();
        }

        //Deserialize it
        void Deserialize(const std::string& validJSONName)
        {
            //Create a document first
            Document document;

            //Parse JSON into string validJSONName, return false if fail
            if (!InitDocument(validJSONName, document))
            {
                std::cout << "Parsing of JSON into string failed" << std::endl;
                // Do some assert
            }

            Deserialize(Reader{ document });
        }

        // Default Deserialize Function
        void Deserialize(Reader reader) override
        {
            //std::vector<std::string> dogVector;
            //reader.GetContainerValue(dogVector, "DogVector");
            //for (auto itr : dogVector)
            //{
            //    std::cout << itr.c_str() << std::endl;
            //}

            //std::map<std::string, int> cowMap;
            //reader.GetContainerValue(cowMap, "CowMap");
            //for (auto itr : cowMap)
            //{
            //    std::cout << itr.first << std::endl;
            //    std::cout << itr.second << std::endl;
            //}

            //std::string s = "alllah";
            //int allahValue = reader.GetValue<int>(s);
            //std::cout << allahValue << std::endl;
        }
    private:
    };


    // *********************************************************
    // *Exposed Serialize Functions to serialize data to a JSON file
    // *********************************************************
    std::string ToJsonFormat(const instance& obj)
    {
        if (!obj.is_valid())
        {
            std::cout << "RTTR object is not valid!" << std::endl;
            return std::string();
        }

        StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        Writer ownWriter{ writer };
        ownWriter.WriteToJSONRecursively(obj);
        return sb.GetString();
    }

    void SerializeToFile(const std::filesystem::path& filePath, const instance& obj)
    {
        std::string JSONStringBuffer = ToJsonFormat(obj);
        std::ofstream file{ filePath };
        // Check if file is in good bit
        if (file.good())
        {
            // Retrieve the strings from JSONStringBuffer
            file << JSONStringBuffer;
        }
        else
        {
            // Do some Assert to show SerializeToFile failed
        }
    }

    // *********************************************************
    // *Functions to get value out of JSON VALUE type
    // *********************************************************
    template <typename Type>
    Type GetValue(const Value& value)
    {
        if constexpr (TYPETRAITS::are_same<Type, uint64_t >::value)
        {
            return value.GetUint64();
        }
        if constexpr (TYPETRAITS::are_same<Type, int64_t >::value)
        {
            return value.GetInt64();
        }
        if constexpr (TYPETRAITS::are_same<Type, double>::value)
        {
            return value.GetDouble();
        }
        if constexpr (TYPETRAITS::are_same<Type, float>::value)
        {
            return value.GetFloat();
        }
        if constexpr (TYPETRAITS::are_same<Type, bool>::value)
        {
            return value.GetBool();
        }
        if constexpr (TYPETRAITS::are_same<Type, int >::value)
        {
            return value.GetInt();
        }
        if constexpr (TYPETRAITS::are_same<Type, unsigned >::value)
        {
            return value.GetUint();
        }
        if constexpr (TYPETRAITS::are_same<Type, std::string>::value || TYPETRAITS::are_same<Type, const char*>::value)
        {
            return value.GetString();
        }
        std::cerr << "Unable to return a type" << std::endl;
        throw 0;
    }

    template <typename Type>
    Type GetType(const Value& value)
    {
        return value.GetType();
    }

    /*bool QueryValue(Document document, const std::string& value)
    {
        assert(!document.IsNull());
        if (!document.IsObject())
        {
            std::cout << "Document is not an object!" << std::endl;
            return false;
        }

        if (value.empty())
        {
            std::cout << "Value is empty" << std::endl;
            return false;
        }

        if (document.HasMember(value.c_str()) == false)
        {
            std::cout << "Document does not contain " << value.c_str() << " this member" << std::endl;
            return false;
        }

        if (document.Parse(value.c_str()).HasParseError())
        {
            std::cout << "Document has parse error" << std::endl;
            return false;
        }

        return true;
    }
    */
}

#endif