/***
    mockcpp is a generic C/C++ mock framework.
    Copyright (C) <2009>  <Darwin Yuan: darwin.yuan@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***/

#ifndef __MOCKCPP_H
#define __MOCKCPP_H

#ifdef _MSC_VER
#define _MOCKCPP_COMPILER_MSVC  _MSC_VER
#elif defined(__GNUC__)
#define _MOCKCPP_COMPILER_GNUC
#else
#error "Unknown compiler!"
#endif

#if !defined(MOCKCPP_NO_NAMESPACE) || (MOCKCPP_NO_NAMESPACE == 0)
# define MOCKCPP_NS mockcpp
# define MOCKCPP_NS_START namespace MOCKCPP_NS {
# define MOCKCPP_NS_END }
# define USING_MOCKCPP_NS using namespace MOCKCPP_NS;
#else
# define MOCKCPP_NS 
# define MOCKCPP_NS_START 
# define MOCKCPP_NS_END 
# define USING_MOCKCPP_NS 
#endif

#ifdef _MSC_VER
# define MOCKCPP_EXPORT __declspec(dllexport)
#else
# define MOCKCPP_EXPORT 
#endif

#if  ( defined (__aarch64__))

#define BUILD_FOR_AARCH64

#elif  ( defined (__arm__))

#define BUILD_FOR_AARCH32

#elif  ( defined (__LP64__) \
    || defined (__64BIT__) \
    || defined (_LP64) \
    || ((defined(__WORDSIZE)) && (__WORDSIZE == 64)) \
    || defined(_WIN64))

#define BUILD_FOR_X64

#else

#define BUILD_FOR_X86

#endif

MOCKCPP_NS_START

namespace __CHECK {

  template<bool condition>
  struct STATIC_ASSERT_FAILURE;

  template<> struct STATIC_ASSERT_FAILURE<true>
  {
    enum { value = 1 };
  };

  template<int x> struct STATIC_ASSERT_TEST;

#define MOCKCPP_STATIC_ASSERT(C) \
  typedef MOCKCPP_NS::__CHECK::STATIC_ASSERT_TEST< \
    sizeof(MOCKCPP_NS::__CHECK::STATIC_ASSERT_FAILURE<!!(C)>)> \
    mockcpp_static_assert_typedef
}

MOCKCPP_NS_END
#endif // __MOCKCPP_H

