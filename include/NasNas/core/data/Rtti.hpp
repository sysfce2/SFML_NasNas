// Created by Modar Nasser on 30/07/2021.

#pragma once

namespace rtti {

#define DEFINE_HAS_METHOD(name, signature)                                      \
    template<typename, typename T=signature>                                    \
    struct has_##name {};                                                       \
    template<typename C, typename Ret, typename... Args>                        \
    struct has_##name<C, Ret(Args...)> {                                        \
    private:                                                                    \
        template <typename Rt>                                                  \
        using remove_const_ref = std::remove_const_t<std::remove_reference_t<Rt>>;\
        template<typename T>                                                    \
        static constexpr auto check(T*) ->                                      \
            std::enable_if_t<std::is_base_of_v<                                 \
                remove_const_ref<decltype(std::declval<T>().name(std::declval<Args>()...))>,\
                remove_const_ref<Ret>                                           \
            >, std::true_type>;                                                 \
        template<typename> static constexpr std::false_type check(...);         \
        using type = decltype(check<C>(0));                                     \
    public:                                                                     \
        static constexpr bool value = type::value;                              \
    };                                                                          \
    template <typename C, typename T=signature>                                 \
    inline constexpr bool has_##name##_v = has_##name<C, T>::value

    DEFINE_HAS_METHOD(getPosition, sf::Vector2f());
    DEFINE_HAS_METHOD(getGlobalBounds, sf::FloatRect());
    DEFINE_HAS_METHOD(getBounds, sf::FloatRect());
    DEFINE_HAS_METHOD(update, void());
}
