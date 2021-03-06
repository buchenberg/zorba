/*
 * Copyright 2006-2008 The FLWOR Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once
#ifndef ZORBA_STATIC_CONTEXT_CONSTS_H
#define ZORBA_STATIC_CONTEXT_CONSTS_H

#include <string>
#include <iostream>

namespace zorba 
{

namespace StaticContextConsts 
{

  enum language_kind_t
  {
    language_kind_unknown = 0,
    language_kind_xquery,
    language_kind_jsoniq
  };

  enum xquery_version_t
  {
    xquery_version_unknown = 0,
    xquery_version_1_0 = 100,
    xquery_version_3_0 = 300,
  };

  enum jsoniq_version_t
  {
    jsoniq_version_unknown = 0,
    jsoniq_version_1_0 = 100,
  };

  std::ostream& operator<<(std::ostream& o, xquery_version_t v);

  std::ostream& operator<<(std::ostream& o, jsoniq_version_t v);


  enum xpath_compatibility_t 
  {
    xpath_unknown,
    xpath2_0,
    xpath1_0_only
  };

  enum construction_mode_t   
  {
    cons_unknown,
    cons_preserve,
    cons_strip
  };

  enum ordering_mode_t
  {
    ordering_unknown,
    ordered,
    unordered 
  };

  enum empty_order_mode_t
  {
    empty_order_unknown,
    empty_greatest,
    empty_least
  };

  enum boundary_space_mode_t
  {
    boundary_space_unknown,
    preserve_space, 
    strip_space 
  };

  enum validation_mode_t
  {
    validation_unknown,
    strict_validation,
    lax_validation,
    lax_dtd_validation,
    skip_validation
  };

  enum declaration_property_t
  {
    // Collection update mode
    decl_const,        
    decl_append_only,
    decl_queue,
    decl_mutable,

    // Collection ordering
    decl_ordered,
    decl_unordered,

    // Index Ordering
    decl_value_equality,
    decl_value_range,
    decl_general_equality,
    decl_general_range,

    // Index uniqueness
    decl_unique,       
    decl_non_unique,

    // index maintenance mode
    decl_automatic,    
    decl_manual
  };


  enum node_modifier_t
  {
    read_only,
    mutable_node
  };
  

  std::string toString(declaration_property_t prop);

} // namespace StaticContextConsts
} // namespace zorba

#endif /* ZORBA_STATIC_CONTEXT_CONSTS_H */
/*
 * Local variables:
 * mode: c++
 * End:
 */
/* vim:set et sw=2 ts=2: */
