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

#ifndef __MOCKCPP_GLOBAL_MOCK_OBJECT_H
#define __MOCKCPP_GLOBAL_MOCK_OBJECT_H

#include <mockcpp/mockcpp.h>

#if defined(MOCKCPP_USE_MOCKABLE) 
#  include <mockcpp/ChainableMockObject.h>
#else
#  include <mockcpp/HookMockObject.h>
#endif

MOCKCPP_NS_START

#if defined(MOCKCPP_USE_MOCKABLE) 
typedef ChainableMockObject MockObjectType;
#else
typedef HookMockObject MockObjectType;
#endif

struct GlobalMockObject
{
   static void verify();
   static void reset();
   static void reset(const void* api);

   static MockObjectType instance;
};

namespace Details {
  template<typename Method>
  void *methodToAddr(Method m)
  {
    union
    {
      void *addr_;
      Method m_;
    };

    m_ = m;
    return addr_;
  }
}  // namespace Details

MOCKCPP_NS_END

#endif

