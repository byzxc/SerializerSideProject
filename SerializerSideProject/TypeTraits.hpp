/******************************************************************************/
/*!
\file       TypeTraits.hpp
\author     Darren Lin (100% code contribution)
\copyright  Copyright (C) 2021 DigiPen Institute of Technology. Reproduction
            or disclosure of this file or its contents without the prior
            written consent of DigiPen Institute of Technology is prohibited.
 */
 /******************************************************************************/
#ifndef _TYPETRAITS_HPP_
#define _TYPETRAITS_HPP_

#include <algorithm>
#include <iostream>
#include <type_traits>
#include <utility>

namespace TYPETRAITS
{
    // ************************************************
    // Two templated structs to help me check if the container contains Pair
    // ************************************************
    template <typename>
    struct is_pair : std::false_type {};

    template <typename Key, typename Value>
    struct is_pair<std::pair<Key, Value>> : std::true_type {};
    // ************************************************

    // ************************************************
    // Templated structs to help me check if the container is tuple
    // ************************************************
    template <typename... Args>
    struct is_tuple : std::false_type {};

    template <typename... Args>
    struct is_tuple<std::tuple<Args...>> : std::true_type {};
    // ************************************************

    // ************************************************
    // Build a parameter pack of numbers and unpack them using template meta-programming
    // ************************************************
    template<int ...>
    struct seq {};

    // It will recursively call itself to become gens<0, 0, 1, 2>
    template<int N, int ...S>
    struct gens : gens<N - 1, N - 1, S...> { };

    template<int ...S>
    struct gens<0, S...>
    {
        // seq<S...> type is essentially typedef seq<0, 1, 2> type if your N is 3
        typedef seq<S...> type;
    };
    // ************************************************

    // ************************************************
    // Templated structs to check if all the arguments inside the parameter packs are of the same type
    // ************************************************
    template<typename... Args>
    struct same : std::true_type {};

    template<typename Arg, typename... Args>
    struct same<Arg, Args...> : std::conditional<Arg::value, same<Args...>, std::false_type>::type {};

    template<typename Arg, typename... Args>
    using are_same = same<std::is_same<Args, Arg>...>;
    // ************************************************

    // ************************************************
    // Typenamed ....
    // ************************************************
    template< bool B, class T, class F >
    using conditional_t = typename std::conditional<B, T, F>::type;

    template<typename T>
    using remove_cv_t = typename std::remove_cv<T>::type;

    template<typename T>
    using remove_volatile_t = typename std::remove_volatile<T>::type;

    template<typename T>
    using remove_const_t = typename std::remove_const<T>::type;

    template<typename T>
    using remove_pointer_t = typename std::remove_pointer<T>::type;

    template<typename T>
    using remove_reference_t = typename std::remove_reference<T>::type;

    template<typename T>
    using add_pointer_t = typename std::add_pointer<T>::type;

    template< bool B, class T = void >
    using enable_if_t = typename std::enable_if<B, T>::type;

    template<typename T>
    using decay_t = typename std::decay<T>::type;

    template<typename T>
    using add_const_t = typename std::add_const<T>::type;

    template<typename T>
    using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

}

#endif