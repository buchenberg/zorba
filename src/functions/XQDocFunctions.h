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
#ifndef XQDOC_FUNCTIONS_H
#define XQDOC_FUNCTIONS_H

#include "common/shared_types.h"
#include "functions/function.h"

namespace zorba {

class XQDocFunction: public function
{
  public:
    XQDocFunction(const signature& sig): function(sig){}

    PlanIter_t codegen (short sctx,
                        const QueryLoc& loc,
                        std::vector<PlanIter_t>& argv,
                        AnnotationHolder &ann) const;
};
}//end of namespace
#endif
