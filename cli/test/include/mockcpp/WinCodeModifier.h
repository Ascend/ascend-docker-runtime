/***
    mockcpp is a generic C/C++ mock framework.
    Copyright (C) <2010>  <Darwin Yuan: darwin.yuan@gmail.com>
	                      <Chen Guodong: sinojelly@gmail.com>

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

#ifndef __MOCKCPP_WINCODEMODIFIER_H__
#define __MOCKCPP_WINCODEMODIFIER_H__

#include <mockcpp/CodeModifier.h>

MOCKCPP_NS_START

struct WinCodeModifier : public CodeModifier
{
	bool modify(void *dest, void *src, size_t size);
};


MOCKCPP_NS_END

#endif
