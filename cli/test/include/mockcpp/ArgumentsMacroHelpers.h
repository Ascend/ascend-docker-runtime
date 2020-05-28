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

#ifndef __MOCKCPP_ARGUMENTS_MACRO_HELPERS_H
#define __MOCKCPP_ARGUMENTS_MACRO_HELPERS_H

#include <mockcpp/RepeatMacros.h>

#define __ARG(n) MOCKP ## n
#define END_ARGS(n) __ARG(n)
#define ARGS(n) __ARG(n) ,
#define REST_ARGS(n) , __ARG(n)

#define __PARAM(n) p ## n
#define END_PARAMS(n) __PARAM(n)
#define PARAMS(n) __PARAM(n) ,
#define REST_PARAMS(n) , __PARAM(n)

#define TEMPLATE_ARG(n) typename __ARG(n)
#define TEMPLATE_ARGS(n) , TEMPLATE_ARG(n)
#define VOID_TEMPLATE_ARGS(n) typename __ARG(n),

#define __ARG_DECL(n) __ARG(n) __PARAM(n)
#define ARG_DECL(n) __ARG_DECL(n) ,
#define END_ARG_DECL(n) __ARG_DECL(n)
#define REST_ARG_DECL(n) , __ARG_DECL(n)

#define DECL_TEMPLATE_ARGS(n) SIMPLE_REPEAT(n, TEMPLATE_ARGS)
#define DECL_VARDIC_ARGS(n) SIMPLE_REPEAT(n, ARGS)
#define DECL_VARDIC_PARAM_LIST(n) SIMPLE_REPEAT(n, ARG_DECL)
#define DECL_VOID_TEMPLATE_ARGS(n) REPEAT(n, VOID_TEMPLATE_ARGS, TEMPLATE_ARG)
#define DECL_ARGS(n) REPEAT(n, ARGS, END_ARGS)
#define DECL_PARAMS(n) REPEAT(n, PARAMS, END_PARAMS)
#define DECL_PARAMS_LIST(n) REPEAT(n, ARG_DECL, END_ARG_DECL)

#define DECL_REST_ARGS(n) SIMPLE_REPEAT(n, REST_ARGS)
#define DECL_REST_PARAMS(n) SIMPLE_REPEAT(n, REST_PARAMS)
#define DECL_REST_ARG_DECL(n) SIMPLE_REPEAT(n, REST_ARG_DECL)

#define MOCKER_PP_CAT(a, b)  MOCKER_PP_CAT_I(a, b)
#define MOCKER_PP_CAT_I(a, b)  MOCKER_PP_CAT_II(~, a ## b)
#define MOCKER_PP_CAT_II(p, res)  res

#define MOCKER_PP_VARIADIC_SIZE(...)  MOCKER_PP_VARIADIC_SIZE_I(__VA_ARGS__)
#define MOCKER_PP_VARIADIC_SIZE_I(...)  MOCKER_PP_VARIADIC_SIZE_II(MOCKER_PP_VARIADIC_SIZE_S(__VA_ARGS__, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1,))
#define MOCKER_PP_VARIADIC_SIZE_II(res)  res
#define MOCKER_PP_VARIADIC_SIZE_S(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, e34, e35, e36, e37, e38, e39, e40, e41, e42, e43, e44, e45, e46, e47, e48, e49, e50, e51, e52, e53, e54, e55, e56, e57, e58, e59, e60, e61, e62, e63, size, ...)  size

#define MOCKER_PP_OVERLOAD(prefix, ...)  MOCKER_PP_CAT(prefix, MOCKER_PP_VARIADIC_SIZE(__VA_ARGS__))

#endif

