#pragma once

#ifndef DXFEED_HPP_INCLUDED
#    error Please include only the DXFeed.hpp header
#endif

#ifndef DXFCPP_NOEXCEPT
#    if __cplusplus >= 201103L
#        define DXFCPP_NOEXCEPT         noexcept
#        define DXFCPP_NOEXCEPT_IF(...) noexcept(__VA_ARGS__)
#        define DXFCPP_USE_NOEXCEPT     noexcept
#        define DXFCPP_THROW(EXC)
#    else
#        define DXFCPP_NOEXCEPT
#        define DXFCPP_NOEXCEPT_IF(...)
#        define DXFCPP_USE_NOEXCEPT throw()
#        define DXFCPP_THROW(EXC)   throw(EXC)
#    endif
#endif

#ifndef DXFCPP_CONSTEXPR
#    if __cplusplus >= 201103L
#        define DXFCPP_CONSTEXPR     constexpr
#        define DXFCPP_USE_CONSTEXPR constexpr
#    else
#        define DXFCPP_CONSTEXPR
#        define DXFCPP_USE_CONSTEXPR const
#    endif
#endif