#ifndef _TYPETRAITS_HPP_
#define _TYPETRAITS_HPP_

#include <type_traits>
#include <utility>
#include "rttr/type"


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
}

#endif