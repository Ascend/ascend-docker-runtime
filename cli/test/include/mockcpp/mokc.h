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

#ifndef __MOCKCPP_MOKC_H
#define __MOCKCPP_MOKC_H

#ifdef __cplusplus
#  include <mockcpp/ChainingMockHelper.h>
#  include <mockcpp/ProcStub.h>

# if defined(MOCKCPP_USE_MOCKABLE) 
#   include <mockcpp/Functor.h>
#   define MOCKER(api) MOCKCPP_NS::GlobalMockObject::instance.method(#api)
# else
#   include <mockcpp/ApiHookMocker.h>
// @TODO : write instructions
// @TODO : update mockcpp
//#   define __MOCKER_MODE_1(api) MOCKCPP_NS::mockAPI<>::get(#api, "", api)
//#   define __MOCKER_MODE_2(api, API) MOCKCPP_NS::mockAPI<API>::get(#api, #API, api)
//#   define __MOCKER_CPP_MODE_1(api) MOCKCPP_NS::mockAPI<>::get(#api, "", api)
//#   define __MOCKER_CPP_MODE_2(api, API) MOCKCPP_NS::mockAPI<API>::get(#api, #API, api)
//#   define __MOCKER_CPP_VIRTUAL_MODE_1(obj, api) MOCKCPP_NS::mockAPI<>::get_virtual(#api, "", obj, api)
//#   define __MOCKER_CPP_VIRTUAL_MODE_2(obj, api, API) MOCKCPP_NS::mockAPI<API>::get_virtual(#api, #API, obj, api)
#   define MOCKER(api, ...) MOCKCPP_NS::mockAPI<__VA_ARGS__>::get(#api, ""#__VA_ARGS__, api)
#   define MOCKER_CPP(api, ...) MOCKCPP_NS::mockAPI<__VA_ARGS__>::get(#api, ""#__VA_ARGS__, api)
#   define MOCKER_CPP_VIRTUAL(obj, api, ...) MOCKCPP_NS::mockAPI<__VA_ARGS__>::get_virtual(#api, ""#__VA_ARGS__, obj, api)
// adapter for amock
#   define AMOCKER MOCKER
#   define LLT_GLOBALOBJMOCKER GlobalMockObject
# endif

USING_MOCKCPP_NS

#endif

#endif

