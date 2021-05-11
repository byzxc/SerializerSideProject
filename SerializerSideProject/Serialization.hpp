#ifndef _SERIALIZER_HPP_
#define _SERIALIZER_HPP_

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

namespace JSON
{
	using namespace rapidjson;

	class Writer : private PrettyWriter<StringBuffer>
	{
	public:
		// Default Constructor
		Writer() = default;

		// Constructor
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

		template <typename Type>
		void PutValue(const Type& key)
		{
			if constexpr (TYPETRAITS::areSame<Type, uint64_t >::value)
			{
				m_Writer->Uint64(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, int64_t >::value)
			{
				m_Writer->Int64(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, double>::value || TYPETRAITS::areSame<Type, float>::value)
			{
				m_Writer->Double(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, bool>::value)
			{
				m_Writer->Bool(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, int >::value)
			{
				m_Writer->Int(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, unsigned >::value)
			{
				m_Writer->Uint(key);
			}
			else if constexpr (TYPETRAITS::areSame<Type, std::string>::value || TYPETRAITS::areSame<Type, const char*>::value)
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
		void PutValue(const std::string& key, const Container<Type, Containee...>& valueContainer)
		{
			PutValue(key);
			PutContainerValue(valueContainer);
		}

		//TODO:: Multimap , Multiset not fully tested
		template<typename Type, template<typename, typename...> class Container, typename... Containee>
		void PutContainerValue(const Container<Type, Containee...>& valueContainer)
		{
			StartArray();
			// When dealing with containers, I would want the JSON file to be in one line
			SetFormatOptions(kFormatSingleLineArray);

			if constexpr (TYPETRAITS::is_pair<std::decay_t<decltype(*valueContainer.begin())>>::value)
			{
				for (auto& itr : valueContainer)
				{
					// Safety Check for same Type, unable to StartObject for same type
					if (TYPETRAITS::areSame<Type, Containee...>::value)
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
				for (auto& itr : valueContainer)
				{
					PutValue(itr);
				}
			}
			EndArray();
		}

	private:
		PrettyWriter<StringBuffer>* m_Writer = nullptr;
	};

	//TODO:: Expose some Writer functions in this namespace to allow Serialize
	//       Takes in Write in the function and stuff they want to Serialize....
	template <typename Type>
	void PutValue(Writer& writer, const Type& Value)
	{
		writer.PutValue(Value);
	}

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

			if constexpr (TYPETRAITS::areSame<TValue, uint64_t>::value)
			{
				return value.GetUint64();
			}

			if constexpr (TYPETRAITS::areSame<TValue, int64_t>::value)
			{
				return value.GetInt64();
			}

			if constexpr (TYPETRAITS::areSame<TValue, double>::value)
			{
				return value.GetDouble();
			}

			if constexpr (TYPETRAITS::areSame<TValue, float>::value)
			{
				return value.GetFloat();
			}

			if constexpr (TYPETRAITS::areSame<TValue, bool>::value)
			{
				return value.GetBool();
			}

			if constexpr (TYPETRAITS::areSame<TValue, int>::value)
			{
				return value.GetInt();
			}

			if constexpr (TYPETRAITS::areSame<TValue, unsigned>::value)
			{
				return value.GetUint();
			}

			if constexpr (TYPETRAITS::areSame<TValue, std::string>::value || TYPETRAITS::areSame<TValue, const char*>::value)
			{
				return value.GetString();
			}
		}

		template <typename T, typename... TValue, typename Encoding, typename Allocator = RAPIDJSON_DEFAULT_ALLOCATOR>
		T GetValue(const GenericValue<Encoding, Allocator>& Value)
		{
			if constexpr (TYPETRAITS::areSame<T, uint64_t>::value)
			{
				return Value.GetUint64();
			}

			if constexpr (TYPETRAITS::areSame<T, int64_t>::value)
			{
				return Value.GetInt64();
			}

			if constexpr (TYPETRAITS::areSame<T, double>::value)
			{
				return Value.GetDouble();
			}

			if constexpr (TYPETRAITS::areSame<T, float>::value)
			{
				return Value.GetFloat();
			}

			if constexpr (TYPETRAITS::areSame<T, bool>::value)
			{
				return Value.GetBool();
			}

			if constexpr (TYPETRAITS::areSame<T, int>::value)
			{
				return Value.GetInt();
			}

			if constexpr (TYPETRAITS::areSame<T, unsigned>::value)
			{
				return Value.GetUint();
			}

			if constexpr (TYPETRAITS::areSame<T, std::string>::value || TYPETRAITS::areSame<T, const char*>::value)
			{
				return Value.GetString();
			}
			return {};
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
					for (auto& itr : data.GetArray())
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

	private:
		Value* m_Data = nullptr;
	};

	// This class is for whoever wish to have their data to be serialized. Some examples are components who will inherit this class
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
			// This should be overriden by the class that inherits Serialize //
			std::vector<std::string> dogVector;
			dogVector.push_back("Cb");
			dogVector.push_back("Knn");
			writer.PutValue("DogVector", dogVector);

			std::map<std::string, int> cowMap;
			cowMap.emplace("cow", 1);
			cowMap.emplace("cownnie", 2);
			writer.PutValue("CowMap", cowMap);

			writer.PutKey("alllah");
			writer.PutValue(1);
		}

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
			std::vector<std::string> dogVector;
			reader.GetContainerValue(dogVector, "DogVector");
			for (auto itr : dogVector)
			{
			    std::cout << itr.c_str() << std::endl;
			}

			std::map<std::string, int> cowMap;
			reader.GetContainerValue(cowMap, "CowMap");
			for (auto itr : cowMap)
			{
				std::cout << itr.first << std::endl;
				std::cout << itr.second << std::endl;
			}

			std::string s = "alllah";
			int allahValue = reader.GetValue<int>(s);
			std::cout << allahValue << std::endl;
		}
	private:
	};

	// *********************************************************
	// *Functions to get value out of specific type
	// *********************************************************
	inline int64_t GetInt64(const Value& value)
	{
		return value.GetInt64();
	}

	inline uint64_t GetUint64(const Value& value)
	{
		return value.GetUint64();
	}

	inline int32_t GetInt(const Value& value)
	{
		return value.GetInt();
	}

	inline uint32_t GetUInt(const Value& value)
	{
		return value.GetUint();
	}

	inline float GetFloat(const Value& value)
	{
		return value.GetFloat();
	}

	inline double GetDouble(const Value& value)
	{
		return value.GetDouble();
	}

	template <typename Type>
	Type GetType(const Value& value)
	{
		return value.GetType();
	}

	bool QueryValue(Document document, const std::string& value)
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
}

#endif