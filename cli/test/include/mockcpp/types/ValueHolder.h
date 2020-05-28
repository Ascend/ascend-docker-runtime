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

#ifndef __MOCKPP_VALUEHOLDER_H
#define __MOCKPP_VALUEHOLDER_H

#include <iostream>

#include <mockcpp/mockcpp.h>

#include <mockcpp/types/Holder.h>

#include <mockcpp/EqualityUtil.h>
#include <mockcpp/Formatter.h>
#include <mockcpp/IsEqual.h>
#include <mockcpp/Void.h>
#include <mockcpp/IsAnythingHelper.h>

MOCKCPP_NS_START

namespace {

template <typename T>
Constraint* constraint(const T& value)
{
    return new IsEqual<T>(value);
}

Constraint* constraint(Constraint* c)
{
    return c == 0 ? any() : c;
}

Constraint* constraint(const Constraint* c)
{
    return constraint(const_cast<Constraint*>(c));
}

Constraint* constraint(const Void& v)
{
    return any();
}

}

template<typename ValueType>
struct ValueHolderBase : public Holder<ValueType>
{
    Constraint* getConstraint() const
    {
       return constraint(getValue());
    }

    virtual const ValueType& getValue() const = 0;
};

template<typename ValueType>
struct ValueHolder : public ValueHolderBase<ValueType>
{
    ValueHolder(const ValueType& value)
      : held(value)
    {
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held); }

    const ValueType& getValue() const
    {
      return held;
    }

private:

    ValueType held;
};

///////////////////////////////////////////////
template <typename ValueType>
struct UnsignedLongHolder : public ValueHolderBase<ValueType>
{
protected:
    union Held{
       unsigned long  ul;
       unsigned int   ui;
       unsigned short us;
       unsigned char  uc;
       Held(const unsigned long  value) : ul(value) {}
       Held(const unsigned int   value) : ui(value) {}
       Held(const unsigned short value) : us(value) {}
       Held(const unsigned char  value) : uc(value) {}
    } held;
public:

    UnsignedLongHolder(const ValueType& value)
        : held(value)
    {
    }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<unsigned long> 
   : public UnsignedLongHolder<unsigned long>
{
    ValueHolder(unsigned long value)
      : UnsignedLongHolder<unsigned long>(value)
    {
    }

    const unsigned long& getValue() const
    {
       return held.ul;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.ul); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<unsigned int> 
   : public UnsignedLongHolder<unsigned int>
{
    ValueHolder(unsigned int value)
      : UnsignedLongHolder<unsigned int>(value)
    {
    }

    const unsigned int& getValue() const
    {
       return held.ui;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.ui); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<unsigned short> 
   : public UnsignedLongHolder<unsigned short>
{
    ValueHolder(unsigned short value)
      : UnsignedLongHolder<unsigned short>(value)
    {
    }

    const unsigned short& getValue() const
    {
       return held.us;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.us); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<unsigned char> 
   : public UnsignedLongHolder<unsigned char>
{
    ValueHolder(unsigned char value)
      : UnsignedLongHolder<unsigned char>(value)
    {
    }

    const unsigned char& getValue() const
    {
       return held.uc;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.uc); }
};

///////////////////////////////////////////////
template <typename ValueType>
struct SignedLongHolder : public ValueHolderBase<ValueType>
{
protected:
    union Held{
       long  sl;
       int   si;
       short ss;
       char  sc;
       Held(const long  value) : sl(value) {}
       Held(const int   value) : si(value) {}
       Held(const short value) : ss(value) {}
       Held(const char  value) : sc(value) {}
    } held;
public:

    SignedLongHolder(const ValueType& value)
        : held(value)
    {
    }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<long> 
   : public SignedLongHolder<long>
{
    ValueHolder(const long& value)
      : SignedLongHolder<long>(value)
    {
    }

    const long& getValue() const
    {
       return held.sl;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.sl); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<int> 
   : public SignedLongHolder<int>
{
    ValueHolder(const int& value)
      : SignedLongHolder<int>(value)
    {
    }

    const int& getValue() const
    {
       return held.si;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.si); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<short> 
   : public SignedLongHolder<short>
{
    ValueHolder(const short& value)
      : SignedLongHolder<short>(value)
    {
    }

    const short& getValue() const
    {
       return held.ss;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.ss); }
};

///////////////////////////////////////////////
template <>
struct ValueHolder<char> 
   : public SignedLongHolder<char>
{
    ValueHolder(const char& value)
      : SignedLongHolder<char>(value)
    {
    }

    const char& getValue() const
    {
       return held.sc;
    }

    PlaceHolder * clone() const
    { return new ValueHolder(held.sc); }
};

///////////////////////////////////////////////

MOCKCPP_NS_END


#endif // __MOCKPP_VALUEHOLDER_H


