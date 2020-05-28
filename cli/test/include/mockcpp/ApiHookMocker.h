
#ifndef __MOCKCPP_API_HOOK_MOCKER_H__
#define __MOCKCPP_API_HOOK_MOCKER_H__

#include <mockcpp/mockcpp.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/ApiHookHolderFactory.h>
#include <mockcpp/ArgumentsMacroHelpers.h>
#include <mockcpp/MethodInfoReader.h>
#include <mockcpp/DelegatedMethodGetter.h>

MOCKCPP_NS_START

struct mockAPIauto {};
template<typename API = mockAPIauto> struct mockAPI;

template<typename API> struct mockAPI
{
  static InvocationMockBuilderGetter get(
    const std::string& name, const std::string& type, API api)
  {
    return MOCKCPP_NS::GlobalMockObject::instance.method
      ( type.empty() ? name : name + " #" + type + "#"
      , Details::methodToAddr(api)
      , ApiHookHolderFactory::create(api));
  }

  template<typename C>
  static InvocationMockBuilderGetter get_virtual(
    const std::string& name, const std::string& type, const C *c, API api)
  {
    void ***vtbl = (void ***)c;
    std::pair<unsigned int, unsigned int> indices =
      getIndicesOfMethod<C, API>(api);
    union { void *_addr; API _api; };
    _addr = (*vtbl)[indices.second];
    return MOCKCPP_NS::GlobalMockObject::instance.method
      ( type.empty() ? name : name + " #" + type + "#"
      , _addr
      , ApiHookHolderFactory::create(_api));
  }

  template<typename C>
  static InvocationMockBuilderGetter get_virtual(
    const std::string& name, const std::string& type, const C &c, API api)
  {
    return get_virtual(name, type, &c, api);
  }
};  // struct mockAPI

template<> struct mockAPI<mockAPIauto>
{
#define __MOCKCPP_C_API_GET_FUNCTION_DEF(n, CallingConvention) \
  template<typename R DECL_TEMPLATE_ARGS(n)> \
  static InvocationMockBuilderGetter get( \
    const std::string& name, const std::string& type, R (CallingConvention *api)(DECL_ARGS(n))) \
  { \
    typedef R (CallingConvention *API)(DECL_ARGS(n)); \
    return mockAPI<API>::get(name, type, api); \
  }

#define __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, CallingConvention, ConstMethod) \
  template<typename R, typename C DECL_TEMPLATE_ARGS(n)> \
  static InvocationMockBuilderGetter get( \
    const std::string& name, const std::string& type, R (CallingConvention C::*api)(DECL_ARGS(n)) ConstMethod) \
  { \
    typedef R (CallingConvention C::*API)(DECL_ARGS(n)) ConstMethod; \
    return mockAPI<API>::get(name, type, api); \
  } \
  template<typename R, typename C DECL_TEMPLATE_ARGS(n)> \
  static InvocationMockBuilderGetter get_virtual( \
    const std::string& name, const std::string& type, const C *c, R (CallingConvention C::*api)(DECL_ARGS(n)) ConstMethod) \
  { \
    typedef R (CallingConvention C::*API)(DECL_ARGS(n)) ConstMethod; \
    return mockAPI<API>::get_virtual(name, type, c, api); \
  } \
  template<typename R, typename C DECL_TEMPLATE_ARGS(n)> \
  static InvocationMockBuilderGetter get_virtual( \
    const std::string& name, const std::string& type, const C &c, R (CallingConvention C::*api)(DECL_ARGS(n)) ConstMethod) \
  { \
    typedef R (CallingConvention C::*API)(DECL_ARGS(n)) ConstMethod; \
    return mockAPI<API>::get_virtual(name, type, c, api); \
  }

#ifdef WIN32
#if defined(_MSC_VER) && defined(BUILD_FOR_X86)
#define MOCKCPP_API_GET_FUNCTION_DEF(n) \
  __MOCKCPP_C_API_GET_FUNCTION_DEF(n, ); \
  __MOCKCPP_C_API_GET_FUNCTION_DEF(n, __stdcall); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , const); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, __stdcall, ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, __stdcall, const)
#else
#define MOCKCPP_API_GET_FUNCTION_DEF(n) \
  __MOCKCPP_C_API_GET_FUNCTION_DEF(n, ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , const)
#endif
#else
#define MOCKCPP_API_GET_FUNCTION_DEF(n) \
  __MOCKCPP_C_API_GET_FUNCTION_DEF(n, ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , ); \
  __MOCKCPP_CXX_API_GET_FUNCTION_DEF(n, , const)
#endif

  MOCKCPP_API_GET_FUNCTION_DEF(0);
  MOCKCPP_API_GET_FUNCTION_DEF(1);
  MOCKCPP_API_GET_FUNCTION_DEF(2);
  MOCKCPP_API_GET_FUNCTION_DEF(3);
  MOCKCPP_API_GET_FUNCTION_DEF(4);
  MOCKCPP_API_GET_FUNCTION_DEF(5);
  MOCKCPP_API_GET_FUNCTION_DEF(6);
  MOCKCPP_API_GET_FUNCTION_DEF(7);
  MOCKCPP_API_GET_FUNCTION_DEF(8);
  MOCKCPP_API_GET_FUNCTION_DEF(9);
  MOCKCPP_API_GET_FUNCTION_DEF(10);
  MOCKCPP_API_GET_FUNCTION_DEF(11);
  MOCKCPP_API_GET_FUNCTION_DEF(12);
};  // struct mockAPI<mockAPIauto>

MOCKCPP_NS_END

#endif

