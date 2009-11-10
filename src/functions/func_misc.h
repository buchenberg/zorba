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
 
// ******************************************
// *                                        *
// * THIS IS A GENERATED FILE. DO NOT EDIT! *
// * SEE .xml FILE WITH SAME NAME           *
// *                                        *
// ******************************************


#ifndef ZORBA_FUNCTIONS_MISC_H
#define ZORBA_FUNCTIONS_MISC_H


#include "common/shared_types.h"
#include "functions/function_impl.h"


namespace zorba{


void populate_context_misc(static_context* sctx);




//fn:trace
class fn_trace : public function
{
public:
  fn_trace(const signature& sig)
    :
    function(sig, FunctionConsts::FN_TRACE_2)
  {
  }

  CODEGEN_DECL();
};


//op-zorba:read-line
class op_zorba_read_line : public function
{
public:
  op_zorba_read_line(const signature& sig)
    :
    function(sig, FunctionConsts::OP_ZORBA_READ_LINE_0)
  {
  }

  bool requires_dyn_ctx() const { return true; }

  CODEGEN_DECL();
};


//op-zorba:print
class op_zorba_print : public function
{
public:
  op_zorba_print(const signature& sig)
    :
    function(sig, FunctionConsts::OP_ZORBA_PRINT_1)
  {
  }

  bool requires_dyn_ctx() const { return true; }

  CODEGEN_DECL();
};


//fn:resolve-uri
class fn_resolve_uri : public function
{
public:
  fn_resolve_uri(const signature& sig)
    :
    function(sig, FunctionConsts::FN_RESOLVE_URI_2)
  {
  }

  bool requires_dyn_ctx() const { return true; }

  CODEGEN_DECL();
};


//fn:error
class fn_error : public function
{
public:
  fn_error(const signature& sig);

  bool isFnError() const { return true; }

  expr_script_kind_t getUpdateType() const { return VACUOUS_EXPR; }

  CODEGEN_DECL();
};


} //namespace zorba


#endif
/*
 * Local variables:
 * mode: c++
 * End:
 */ 
