/******************************************************************************/
/*!
\file       Reflect.h
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#ifndef _REFLECT_H_
#define _REFLECT_H_

#include "rttr/registration.h"
#include "rttr/type.h"
#include <vector>
#include <iostream>

#include "SpaceAssert.h"

/*  This namespace contains RTTR_REGISTRATION that is suppose to help register all classes, variables, functions that you would like to
    be able to modify/introspect them in runtime.

    Put RTTR_ENABLE() macro inside every class, where you need the complete inheritance information about the class type.
    When there is no inheritance graph, the macro is not needed.
    Example:
    struct Base
    {
         RTTR_ENABLE()
    };

    struct Derived : Base
    {
        RTTR_ENABLE(Base)
    };

    struct MultipleDerived : Base, Other
    {
        RTTR_ENABLE(Base, Other)
    };

    If you do not want to serialize a variable
    Use (rttr::metadata("NO_SERIALIZE", true));

    Use (rttr::policy::prop::bind_as_ptr) when trying to bind a member object
    as pointer type to avoid copies during get/set of the property
    Example of uses for this is for containers/ object(Vector3)

 */

namespace Reflect
{
    using namespace rttr;

    inline float sin(float x)
    {
        return x * x;
    }

    enum class color
    {
        red,
        green,
        blue
    };

    struct point2d
    {
        point2d() {}
        point2d(int x_, int y_) : x(x_), y(y_) {}
        int x = 0;
        int y = 0;
    };

    struct shape
    {
    public:
        shape(std::string n) : name(n) {}

        void set_visible(bool v) { visible = v; }
        bool get_visible() const { return visible; }

        color color_ = color::blue;
        std::string name = "";
        point2d position;
        std::map<color, point2d> dictionary;

        RTTR_ENABLE()
    private:
        bool visible = false;
    };

    struct Vector3
    {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
    };

    struct circle : shape
    {
        circle(std::string n) : shape(n) {}

        double radius = 5.2;
        std::vector<point2d> points;
        std::vector<int> clown;
        Vector3 allah = { 1.f, 2.f, 3.f };

        // Does not work for tuple
        //std::tuple<int, float> dick{ 1, 2.f };

        int no_serialize = 100;

        RTTR_ENABLE(shape)
    };

    // For registering functions with same name but different signature
    // TODO:: Serialize functions
    template <typename Signature, typename ClassType>
    auto RegisterOverloadedFunctions(Signature(ClassType::* function)) ->decltype(function)
    {
        return function;
    }

    inline std::vector<variant> InvokeGlobalRegisteredFunction(string_view nameOfFunction, std::vector<argument> args)
    {
        method meth = type::get_global_method(nameOfFunction);

        // Variant will be valid if there are values inside it, else it will be invalid
        std::vector<variant> valuesAfterInvokingFunction;

        // Only invoke function if function is found
        if (meth)
        {
            for (const argument& itr : args)
            {
                valuesAfterInvokingFunction.emplace_back(type::invoke(nameOfFunction, { itr }));
            }
        }
        return valuesAfterInvokingFunction;
    }

    /*!
     * \brief Invokes a global method named \p name with the specified argument \p args.
     *
     * \return A variant object containing the possible return value,
     *         otherwise when it is a void function an empty but valid variant object.
     *         Methods with registered \ref default_arguments will be honored.
     */
    template <typename TObject>
    std::vector<variant> InvokeRegisteredFunction(string_view nameOfFunction, std::vector<argument> args)
    {
        const type t = type::get<TObject>();
        std::vector<variant> valueAfterInvokingFunction{};

        // Find the function in the TObject class
        const method meth = t.get_method(nameOfFunction);

        // If function is found
        if (meth)
        {
            for (const auto& itr : args)
            {
                // Invoke with empty instance
                //variant returnValue = meth.invoke(nameOfFunction, itr);
                //if (returnValue.is_valid() && returnValue.is_type<float>())
                //{
                //    std::cout << returnValue.get_value<float>() << std::endl;
                //}
                valueAfterInvokingFunction.emplace_back(meth.invoke(nameOfFunction, itr));
            }
        }

        return valueAfterInvokingFunction;
    }

    template <typename Type>
    Type GetValueFromVariant(variant value)
    {
        //SPACEASSERT(value.is_valid(), "Variant does not contain any data/value");
        //SPACEASSERT(value.is_type<Type>(), "Variant does not match the type provided");
        if (value.is_valid() && value.is_type<Type>())
        {
            return value.get_value<Type>();
        }
        throw 0;
    }
}

#endif
