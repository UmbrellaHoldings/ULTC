//-*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Type information routines.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_TYPEINFO_H
#define COHORS_TYPES_TYPEINFO_H

#include <typeinfo>
#ifndef _WIN32
#include <cxxabi.h>
#endif

namespace types {

//! Return the demangled Type name
template<class Type>
struct type
{
  static std::string name()
  {
#ifndef _WIN32
    // Demangle the name by the ABI rules
    int status;
    const char* mangled = typeid(Type).name();
    char* name = abi::__cxa_demangle
      (mangled, nullptr, nullptr, &status);
    if (status == 0) {
      std::string res(name);
      free(name);
      return res;
    }
    else {
      assert(name == nullptr);
      return std::string(mangled);
    }
#else
    return typeid(Type).name();
#endif
  }
};

} // types

#endif
