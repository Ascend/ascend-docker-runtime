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

#ifndef __MOCKCPP_INVOCATION_H
#define __MOCKCPP_INVOCATION_H

#include <string>

#include <mockcpp/mockcpp.h>

#include <mockcpp/types/RefAny.h>

MOCKCPP_NS_START

struct Stub;
struct InvocationImpl;

struct Invocation
{
    Invocation(const std::string nameOfCaller
             , const RefAny& pThisPointer = RefAny()
             , const RefAny& p01 = RefAny()
             , const RefAny& p02 = RefAny()
             , const RefAny& p03 = RefAny()
             , const RefAny& p04 = RefAny()
             , const RefAny& p05 = RefAny()
             , const RefAny& p06 = RefAny()
             , const RefAny& p07 = RefAny()
             , const RefAny& p08 = RefAny()
             , const RefAny& p09 = RefAny()
             , const RefAny& p10 = RefAny()
             , const RefAny& p11 = RefAny()
             , const RefAny& p12 = RefAny()
    );

    virtual ~Invocation();

    RefAny& getThisPointer(void) const;

    // i is beginning with 1
    RefAny& getParameter(const unsigned int i) const;

    // return in follow order:
    // 'this' pointer(if it is exist), p1, p2, p3, p4 ... pN
    RefAny& getParameterWithThis(const unsigned int i) const;

    std::string getNameOfCaller() const;

    std::string toString(void) const;

private:
    InvocationImpl* This;
};

MOCKCPP_NS_END

#endif

