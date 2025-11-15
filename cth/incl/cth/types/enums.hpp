#pragma once
#include <type_traits>

#define CTH_GEN_ENUM_OVERLOADS(EnumType)\
constexpr auto operator*(EnumType val) {\
    static_assert(requires() { typename std::underlying_type_t<EnumType>; }, "CTH_ENUM_OVERLOADS can only be used with an enum class");\
    return static_cast<std::underlying_type_t<EnumType>>(val);\
}\
\
constexpr EnumType operator|(EnumType const& l, EnumType const& r) { return static_cast<EnumType>(*l | *r); }\
\
constexpr EnumType operator&(EnumType const& l, EnumType const& r) { return static_cast<EnumType>(*l & *r); }\
\
constexpr EnumType operator~(EnumType const& val) { return static_cast<EnumType>(~*val); }\
\
constexpr EnumType& operator|=(EnumType& l, EnumType const& r) { return l = l | r; }\
\
constexpr EnumType& operator&=(EnumType& l, EnumType const& r) {  return l = l & r;}
