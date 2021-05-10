#ifndef _TYPETRAITS_HPP_
#define _TYPETRAITS_HPP_

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
    // Templated structs to check if all the arguments inside the parameter packs are of the same type
    // ************************************************
    template<typename... Args>
    struct same : std::true_type {};

    template<typename Arg, typename... Args>
    struct same<Arg, Args...> : std::conditional<Arg::value, same<Args...>,
                                std::false_type>::type {};

    template<typename Arg, typename... Args>
    using areSame = same<std::is_same<Args, Arg>...>;
    // ************************************************
}

#endif