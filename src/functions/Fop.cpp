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
#include "functions/Fop.h"
#include "functions/function_impl.h"

#include "runtime/fop/FopImpl.h"

namespace zorba 
{

class zorba_fop : public function 
{
public:
  zorba_fop(const signature& sig) 
    :
    function(sig, FunctionConsts::FN_FOP_1)
  {
  }

  DEFAULT_NARY_CODEGEN(ZorbaFopIterator);
};


void populateContext_FOP(static_context* sctx)
{
#ifdef ZORBA_WITH_FOP
DECL(sctx, zorba_fop,
	 (createQName(ZORBA_FOP_FN_NS, "fn-zorba-fop", "fn-zorba-fop"),
	 GENV_TYPESYSTEM.ITEM_TYPE_ONE,
	 GENV_TYPESYSTEM.ITEM_TYPE_ONE));
#endif
}


}
