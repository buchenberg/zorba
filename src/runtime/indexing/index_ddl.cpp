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
#include "runtime/visitors/planiter_visitor.h"
#include "runtime/indexing/index_ddl.h"
#include "runtime/api/plan_wrapper.h"
#include "runtime/api/plan_iterator_wrapper.h"

#include "system/globalenv.h"

#include "store/api/iterator.h"
#include "store/api/store.h"        // for checking if index exists
#include "store/api/item_factory.h" // for creating pul
#include "store/api/iterator_factory.h" // for creating the probe iterator
#include "store/api/pul.h"
#include "store/api/index.h"

#include "compiler/xqddf/value_index.h"

#include "types/typeimpl.h"
#include "types/typeops.h"
#include "types/casting.h"

#include "context/static_context.h"
#include "context/dynamic_context.h"

namespace zorba 
{


static void checkKeyType(
    const QueryLoc& loc,
    TypeManager* tm,
    const IndexDecl* indexDecl,
    ulong keyNo,
    store::Item_t& searchKey)
{
  xqtref_t searchKeyType = tm->create_value_type(searchKey);
  xqtref_t indexKeyType = (indexDecl->getKeyTypes())[keyNo];

  if (indexKeyType != NULL &&
      !TypeOps::is_subtype(*searchKeyType, *indexKeyType))
  {
    ZORBA_ERROR_LOC_DESC_OSS(XPTY0004, loc,
                             "The type of a search key does not mathch the type"
                             << " of the corresponding index key for index "
                             << indexDecl->getName()->getStringValue()->c_str()
                             << ". The search key has type "
                             << searchKeyType->toString()
                             << " and the expected key type is "
                             << indexKeyType->toString());
  }
  else if (indexKeyType == NULL)
  {
    ZORBA_ASSERT(indexDecl->isGeneral());

    if (indexDecl->isOrdered() &&
        (TypeOps::is_subtype(*searchKeyType, *GENV_TYPESYSTEM.NOTATION_TYPE_ONE) ||
         TypeOps::is_subtype(*searchKeyType, *GENV_TYPESYSTEM.HEXBINARY_TYPE_ONE)))
    {
      ZORBA_ERROR_LOC_DESC_OSS(XPTY0004, loc, 
                               "The type of a search key is not valid for");
    }
  }

#if 0  
  if (!TypeOps::is_equal(*searchKeyType, *indexKeyType))
  {
    // promote the search key to a value whose type is the that of the
    // index key type
    store::Item_t tmp;
    tmp.transfer(searchKey);
    if (! GenericCast::castToAtomic(searchKey, tmp, indexKeyType.getp(), *tm))
    {
      ZORBA_ASSERT(false);
    }
  }
#endif
}


SERIALIZABLE_CLASS_VERSIONS(CreateInternalIndexIterator)
END_SERIALIZABLE_CLASS_VERSIONS(CreateInternalIndexIterator)

SERIALIZABLE_CLASS_VERSIONS(CreateIndexIterator)
END_SERIALIZABLE_CLASS_VERSIONS(CreateIndexIterator)

SERIALIZABLE_CLASS_VERSIONS(DeleteIndexIterator)
END_SERIALIZABLE_CLASS_VERSIONS(DeleteIndexIterator)

SERIALIZABLE_CLASS_VERSIONS(RefreshIndexIterator)
END_SERIALIZABLE_CLASS_VERSIONS(RefreshIndexIterator)

SERIALIZABLE_CLASS_VERSIONS(ValueIndexEntryBuilderIterator)
END_SERIALIZABLE_CLASS_VERSIONS(ValueIndexEntryBuilderIterator)

SERIALIZABLE_CLASS_VERSIONS(GeneralIndexEntryBuilderIterator)
END_SERIALIZABLE_CLASS_VERSIONS(GeneralIndexEntryBuilderIterator)

SERIALIZABLE_CLASS_VERSIONS(IndexPointProbeIterator)
END_SERIALIZABLE_CLASS_VERSIONS(IndexPointProbeIterator)

SERIALIZABLE_CLASS_VERSIONS(IndexRangeProbeIterator)
END_SERIALIZABLE_CLASS_VERSIONS(IndexRangeProbeIterator)

SERIALIZABLE_CLASS_VERSIONS(IndexGeneralPointProbeIterator)
END_SERIALIZABLE_CLASS_VERSIONS(IndexGeneralPointProbeIterator)

SERIALIZABLE_CLASS_VERSIONS(IndexGeneralRangeProbeIterator)
END_SERIALIZABLE_CLASS_VERSIONS(IndexGeneralRangeProbeIterator)


/*******************************************************************************

********************************************************************************/
void createIndexSpec(
    IndexDecl* indexDecl,
    store::IndexSpecification& spec)
{
  const std::vector<xqtref_t>& keyTypes(indexDecl->getKeyTypes());
  const std::vector<OrderModifier>& keyModifiers(indexDecl->getOrderModifiers());
  ulong numColumns = keyTypes.size();

  spec.theNumKeyColumns = numColumns;
  spec.resize(numColumns);

  for(ulong i = 0; i < numColumns; ++i) 
  {
    const XQType& t = *keyTypes[i];
    spec.theKeyTypes[i] = t.get_qname();
    spec.theCollations.push_back(keyModifiers[i].theCollation);
  }

  spec.theIsGeneral = indexDecl->isGeneral();
  spec.theIsUnique = indexDecl->getUnique();
  spec.theIsSorted = indexDecl->getMethod() == IndexDecl::TREE;
  spec.theIsTemp = indexDecl->isTemp();
  spec.theIsThreadSafe = true;
  spec.theIsAutomatic = indexDecl->getMaintenanceMode() != IndexDecl::MANUAL;

  ulong numSources = indexDecl->numSources();

  spec.theSources.resize(numSources);

  for (ulong i = 0; i < numSources; ++i)
  {
    spec.theSources[i] = const_cast<store::Item*>(indexDecl->getSourceName(i));
  }
}


/*******************************************************************************
  CreateInternalIndexIterator
********************************************************************************/

CreateInternalIndexIterator::~CreateInternalIndexIterator() 
{
}


bool CreateInternalIndexIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  IndexDecl* indexDecl;
  store::IndexSpecification spec;
  store::Iterator_t planIteratorWrapper;
  store::Index_t storeIndex;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  indexDecl = theSctx->lookup_index(theQName);
  ZORBA_ASSERT(indexDecl);

  planIteratorWrapper = new PlanIteratorWrapper(theChild, planState);

  createIndexSpec(indexDecl, spec);

  try
  {
    storeIndex = GENV_STORE.createIndex(indexDecl->getName(), spec, planIteratorWrapper);
  }
  catch(error::ZorbaError& e)
  {
    // Store raises error if index exists already
    ZORBA_ERROR_LOC_DESC(e.theErrorCode, loc, e.theDescription);
  }

  try
  {
    planState.dctx()->bindIndex(indexDecl->getName(), storeIndex);
  }
  catch(...)
  {
    // Dynamic context raises error if index exists already
    GENV_STORE.deleteIndex(indexDecl->getName());
    throw;
  }

  STACK_END (state);
}


void CreateInternalIndexIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  theChild->accept(v);

  v.endVisit(*this);
}


/*******************************************************************************
  CreateIndexIterator
********************************************************************************/

CreateIndexIterator::~CreateIndexIterator() 
{
}


bool CreateIndexIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t qname;
  IndexDecl_t indexDecl;
  store::IndexSpecification spec;
  PlanIter_t buildPlan;
  store::Iterator_t planWrapper;

  CompilerCB* ccb = planState.theCompilerCB;
  dynamic_context* dctx = planState.dctx();

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(qname, theChild, planState))
    ZORBA_ASSERT(false);

  if ((indexDecl = theSctx->lookup_index(qname)) == NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                          qname->getStringValue()->c_str(), "");
  }

  if (GENV_STORE.getIndex(qname) != NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0022_INDEX_EXISTS_ALREADY, loc,
                          qname->getStringValue()->c_str(), "");
  }

  buildPlan = indexDecl->getBuildPlan(ccb, loc); 
  
  planWrapper = new PlanWrapper(buildPlan, ccb, dctx, NULL); 

  createIndexSpec(indexDecl, spec);

  result = GENV_ITEMFACTORY->createPendingUpdateList();

  reinterpret_cast<store::PUL*>(result.getp())->addCreateIndex(qname, spec, planWrapper);

  STACK_PUSH(true, state);

  STACK_END(state);
}


void CreateIndexIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  theChild->accept(v);

  v.endVisit(*this);
}


/*******************************************************************************
  DeleteIndexIterator
********************************************************************************/

DeleteIndexIterator::~DeleteIndexIterator() 
{
}


bool DeleteIndexIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t qname;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(qname, theChild, planState))
    ZORBA_ASSERT(false);

  if (theSctx->lookup_index(qname) == NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                          qname->getStringValue()->c_str(), "");
  }

  if (GENV_STORE.getIndex(qname) == NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0023_INDEX_DOES_NOT_EXIST, loc,
                          qname->getStringValue()->c_str(), "");
  }

  result = GENV_ITEMFACTORY->createPendingUpdateList();

  reinterpret_cast<store::PUL*>(result.getp())->addDeleteIndex(qname);

  STACK_PUSH(true, state);

  STACK_END(state);
}


void DeleteIndexIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  theChild->accept(v);

  v.endVisit(*this);
}



/*******************************************************************************
  RefreshIndexIterator
********************************************************************************/

RefreshIndexIterator::~RefreshIndexIterator() 
{
}


bool RefreshIndexIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t qname;
  IndexDecl_t indexDecl;
  PlanIter_t buildPlan;
  store::Iterator_t planWrapper;

  dynamic_context* dctx = planState.dctx();
  CompilerCB* ccb = planState.theCompilerCB;

  PlanIteratorState* state;
  DEFAULT_STACK_INIT(PlanIteratorState, state, planState);

  if (!consumeNext(qname, theChild, planState))
    ZORBA_ASSERT(false);

  if ((indexDecl = theSctx->lookup_index(qname)) == NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                          qname->getStringValue()->c_str(), "");
  }

  if (GENV_STORE.getIndex(qname) == NULL)
  {
    ZORBA_ERROR_LOC_PARAM(XDDY0023_INDEX_DOES_NOT_EXIST, loc,
                          qname->getStringValue()->c_str(), "");
  }

  buildPlan = indexDecl->getBuildPlan(ccb, loc); 
  
  planWrapper = new PlanWrapper(buildPlan, ccb, dctx, NULL); 

  result = GENV_ITEMFACTORY->createPendingUpdateList();

  reinterpret_cast<store::PUL*>(result.getp())->addRefreshIndex(qname, planWrapper);

  STACK_PUSH(true, state);

  STACK_END(state);
}


void RefreshIndexIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  theChild->accept(v);

  v.endVisit(*this);
}



/*******************************************************************************
  ValueIndexEntryBuilderIterator
********************************************************************************/

ValueIndexEntryBuilderIteratorState:: ValueIndexEntryBuilderIteratorState() 
{
}


ValueIndexEntryBuilderIteratorState::~ValueIndexEntryBuilderIteratorState() 
{
}


void ValueIndexEntryBuilderIteratorState::init(PlanState& planState) 
{
  PlanIteratorState::init(planState);
  theCurChild = 0;
}


void ValueIndexEntryBuilderIteratorState::reset(PlanState& planState) 
{
  PlanIteratorState::reset(planState);
  theCurChild = 0;
}


ValueIndexEntryBuilderIterator::~ValueIndexEntryBuilderIterator() 
{
}


bool ValueIndexEntryBuilderIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  ValueIndexEntryBuilderIteratorState* state;
  DEFAULT_STACK_INIT(ValueIndexEntryBuilderIteratorState, state, planState);

  for (; state->theCurChild < theChildren.size(); ++state->theCurChild) 
  {
    if (!consumeNext(result, theChildren[state->theCurChild].getp(), planState))
      result = NULL;

    STACK_PUSH(true, state);
  }
  
  STACK_END(state);
}


void ValueIndexEntryBuilderIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
  {
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


/*******************************************************************************
  GeneralIndexEntryBuilderIterator
********************************************************************************/

GeneralIndexEntryBuilderIteratorState::GeneralIndexEntryBuilderIteratorState() 
{
}


GeneralIndexEntryBuilderIteratorState::~GeneralIndexEntryBuilderIteratorState() 
{
}


void GeneralIndexEntryBuilderIteratorState::init(PlanState& planState) 
{
  PlanIteratorState::init(planState);
  theCurChild = 0;
}


void GeneralIndexEntryBuilderIteratorState::reset(PlanState& planState) 
{
  PlanIteratorState::reset(planState);
  theCurChild = 0;
}


GeneralIndexEntryBuilderIterator::~GeneralIndexEntryBuilderIterator() 
{
}


bool GeneralIndexEntryBuilderIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  GeneralIndexEntryBuilderIteratorState* state;
  DEFAULT_STACK_INIT(GeneralIndexEntryBuilderIteratorState, state, planState);

  for (; state->theCurChild < theChildren.size(); ++state->theCurChild) 
  {
    while (consumeNext(result, theChildren[state->theCurChild].getp(), planState))
    {
      STACK_PUSH(true, state);
    }
  }
  
  STACK_END(state);
}


void GeneralIndexEntryBuilderIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
  {
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


/*******************************************************************************
  IndexPointProbeIterator
********************************************************************************/

IndexPointProbeIteratorState::IndexPointProbeIteratorState() 
{
}


IndexPointProbeIteratorState::~IndexPointProbeIteratorState() 
{
}


void IndexPointProbeIteratorState::init(PlanState& planState) 
{
  PlanIteratorState::init(planState);
  theQname = 0;
  theIndexDecl = 0;
  theIndex = 0;
  theIterator = NULL;
}


void IndexPointProbeIteratorState::reset(PlanState& state)
{
  PlanIteratorState::reset(state);
  if (theIterator != NULL) 
  {
    theIterator->close();
  }
}


IndexPointProbeIterator::IndexPointProbeIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& children)
  : 
  NaryBaseIterator<IndexPointProbeIterator,
                   IndexPointProbeIteratorState>(sctx, loc, children),
  theCheckKeyType(true)
{
}


IndexPointProbeIterator::~IndexPointProbeIterator() 
{
}


bool IndexPointProbeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t qnameItem;
  store::Item_t keyItem;
  store::IndexPointCondition_t cond;
  ulong numChildren = theChildren.size();
  ulong i;
  bool status;

  IndexPointProbeIteratorState* state;
  DEFAULT_STACK_INIT(IndexPointProbeIteratorState, state, planState);

  status = consumeNext(qnameItem, theChildren[0], planState);
  ZORBA_ASSERT(status);

  if (state->theQname == NULL || !state->theQname->equals(qnameItem)) 
  {
    state->theQname = qnameItem;

    if ((state->theIndexDecl = theSctx->lookup_index(qnameItem)) == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    if (state->theIndexDecl->getKeyExpressions().size() != numChildren-1)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0025_INDEX_WRONG_NUMBER_OF_PROBE_ARGS, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    state->theIndex = (state->theIndexDecl->isTemp() ?
                       planState.dctx()->getIndex(qnameItem) :
                       GENV_STORE.getIndex(state->theQname));

    if (state->theIndex == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0023_INDEX_DOES_NOT_EXIST, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    state->theIterator = GENV_STORE.getIteratorFactory()->
                         createIndexProbeIterator(state->theIndex);
  }

  cond = state->theIndex->createPointCondition();

  for (i = 1; i < numChildren; ++i) 
  {
    if (!consumeNext(keyItem, theChildren[i], planState)) 
    {
      // We may reach here in the case of internally-generated hashjoins
      break;
    }

    if (keyItem != NULL && theCheckKeyType)
    {
      checkKeyType(loc, theSctx->get_typemanager(), state->theIndexDecl, i-1, keyItem);
    }

    cond->pushItem(keyItem);
  }

  if (i == numChildren)
  {
    state->theIterator->init((const zorba::store::IndexPointCondition_t&)cond);
    state->theIterator->open();

    while(state->theIterator->next(result)) 
    {
      STACK_PUSH(true, state);
    }
  }

  STACK_END(state);
}


void IndexPointProbeIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter ){
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


/*******************************************************************************
  IndexGeneralPointProbeIterator
********************************************************************************/
IndexGeneralPointProbeIteratorState::IndexGeneralPointProbeIteratorState()
{
}


IndexGeneralPointProbeIteratorState::~IndexGeneralPointProbeIteratorState()
{
}


IndexGeneralPointProbeIterator::IndexGeneralPointProbeIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& children)
  : 
  NaryBaseIterator<IndexGeneralPointProbeIterator,
                   IndexGeneralPointProbeIteratorState>(sctx, loc, children),
  theCheckKeyType(true)
{
}


IndexGeneralPointProbeIterator::~IndexGeneralPointProbeIterator() 
{
}


void IndexGeneralPointProbeIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<IndexGeneralPointProbeIterator,
                    IndexGeneralPointProbeIteratorState>*)this);
}


bool IndexGeneralPointProbeIterator::nextImpl(
    store::Item_t& result,
    PlanState& planState) const
{
  store::Item_t qnameItem;
  store::Item_t keyItem;
  ulong numChildren = theChildren.size();
  bool status;

  IndexGeneralPointProbeIteratorState* state;
  DEFAULT_STACK_INIT(IndexGeneralPointProbeIteratorState, state, planState);

  status = consumeNext(qnameItem, theChildren[0], planState);
  ZORBA_ASSERT(status);

  if (state->theQname == NULL || !state->theQname->equals(qnameItem)) 
  {
    state->theQname = qnameItem;

    if ((state->theIndexDecl = theSctx->lookup_index(qnameItem)) == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    if (state->theIndexDecl->getKeyExpressions().size() != numChildren-1 ||
        (state->theIndexDecl->isGeneral() && numChildren != 2))
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0025_INDEX_WRONG_NUMBER_OF_PROBE_ARGS, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    state->theIndex = (state->theIndexDecl->isTemp() ?
                       planState.dctx()->getIndex(qnameItem) :
                       GENV_STORE.getIndex(state->theQname));

    if (state->theIndex == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0023_INDEX_DOES_NOT_EXIST, loc,
                            qnameItem->getStringValue()->c_str(), "");
    }

    state->theIterator = GENV_STORE.getIteratorFactory()->
                         createIndexProbeIterator(state->theIndex);
  }

  if (state->theCondition == NULL)
    state->theCondition = state->theIndex->createPointCondition();

  while (consumeNext(keyItem, theChildren[1], planState)) 
  {
    if (keyItem == NULL)
      // We may reach here in the case of internally-generated hashjoins
      continue;

    if (theCheckKeyType)
    {
      checkKeyType(loc, theSctx->get_typemanager(), state->theIndexDecl, 0, keyItem);
    }

    state->theCondition->clear();
    state->theCondition->pushItem(keyItem);

    state->theIterator->init(state->theCondition.getp());
    state->theIterator->open();

    while (state->theIterator->next(result)) 
    {
      STACK_PUSH(true, state);
    }

    state->theIterator->close();
  }

  STACK_END(state);
}


void IndexGeneralPointProbeIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter ){
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


/*******************************************************************************
  IndexRangeProbeIterator
********************************************************************************/

IndexRangeProbeIteratorState::IndexRangeProbeIteratorState() 
{
}


IndexRangeProbeIteratorState::~IndexRangeProbeIteratorState() 
{
}


void IndexRangeProbeIteratorState::init(PlanState& planState) 
{
  PlanIteratorState::init(planState);
  theQname = 0;
  theIndex = 0;
  theIterator = NULL;
}


void IndexRangeProbeIteratorState::reset(PlanState& state)
{
  PlanIteratorState::reset(state);
  if (theIterator != NULL) 
  {
    theIterator->close();
  }
}


IndexRangeProbeIterator::IndexRangeProbeIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& children)
  : 
  NaryBaseIterator<IndexRangeProbeIterator,
                   IndexRangeProbeIteratorState>(sctx, loc, children),
  theCheckKeyType(true)
{
}


IndexRangeProbeIterator::~IndexRangeProbeIterator() 
{
}


void IndexRangeProbeIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<IndexRangeProbeIterator, IndexRangeProbeIteratorState>*)this);

  ar & theCheckKeyType;
}


bool IndexRangeProbeIterator::nextImpl(store::Item_t& result, PlanState& planState) const
{
  store::Item_t qname;
  IndexDecl_t indexDecl;
  store::IndexBoxCondition_t cond;
  ulong numChildren = theChildren.size();
  bool status;
 
  IndexRangeProbeIteratorState* state;
  DEFAULT_STACK_INIT(IndexRangeProbeIteratorState, state, planState);

  status = consumeNext(qname, theChildren[0], planState);
  ZORBA_ASSERT(status);

  if (state->theQname == NULL || !state->theQname->equals(qname)) 
  {
    state->theQname = qname;

    if ((indexDecl = theSctx->lookup_index(qname)) == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0021_INDEX_IS_NOT_DECLARED, loc,
                            qname->getStringValue()->c_str(), "");
    }

    if (indexDecl->getMethod() != IndexDecl::TREE)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0026_INDEX_RANGE_PROBE_NOT_ALLOWED, loc,
                            qname->getStringValue()->c_str(), "");
    }

    if ((numChildren-1) % 6 != 0)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0025_INDEX_WRONG_NUMBER_OF_PROBE_ARGS, loc,
                            qname->getStringValue()->c_str(), "");
    }

    if (indexDecl->getKeyExpressions().size() * 6 > numChildren-1)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0025_INDEX_WRONG_NUMBER_OF_PROBE_ARGS, loc,
                            qname->getStringValue()->c_str(), "");
    }

    state->theIndex = (indexDecl->isTemp() ?
                       planState.dctx()->getIndex(qname) :
                       GENV_STORE.getIndex(state->theQname));

    if (state->theIndex == NULL)
    {
      ZORBA_ERROR_LOC_PARAM(XDDY0023_INDEX_DOES_NOT_EXIST, loc,
                            qname->getStringValue()->c_str(), "");
    }

    state->theIterator = GENV_STORE.getIteratorFactory()->
                         createIndexProbeIterator(state->theIndex);
  }

  cond = state->theIndex->createBoxCondition();

  ulong keyNo;
  ulong i;
  for (i = 1, keyNo = 0; i < numChildren; i += 6, ++keyNo) 
  {
    store::Item_t tempLeft;
    store::Item_t tempRight;
    store::Item_t tempHaveLeft;
    store::Item_t tempHaveRight;
    store::Item_t tempInclLeft;
    store::Item_t tempInclRight;

    if (!consumeNext(tempLeft, theChildren[i], planState))
      tempLeft = NULL;
 
    if (!consumeNext(tempRight, theChildren[i + 1], planState))
      tempRight = NULL;

    if (!consumeNext(tempHaveLeft, theChildren[i + 2], planState))
      ZORBA_ASSERT(false);

    if (!consumeNext(tempHaveRight, theChildren[i + 3], planState))
     ZORBA_ASSERT(false);

    if (!consumeNext(tempInclLeft, theChildren[i + 4], planState))
     ZORBA_ASSERT(false);
 
    if (!consumeNext(tempInclRight, theChildren[i + 5], planState))
     ZORBA_ASSERT(false);

    bool haveLeft = tempHaveLeft->getBooleanValue();
    bool haveRight = tempHaveRight->getBooleanValue();
    bool inclLeft = tempInclLeft->getBooleanValue();
    bool inclRight = tempInclRight->getBooleanValue();

    if (tempLeft != NULL && theCheckKeyType)
    {
      checkKeyType(loc, theSctx->get_typemanager(), indexDecl, keyNo, tempLeft);
    }

    if (tempRight != NULL && theCheckKeyType)
    {
      checkKeyType(loc, theSctx->get_typemanager(), indexDecl, keyNo, tempRight);
    }

    cond->pushRange(tempLeft, tempRight, haveLeft, haveRight, inclLeft, inclRight);
  }

  state->theIterator->init((const zorba::store::IndexBoxCondition_t&)cond);
  state->theIterator->open();

  while(state->theIterator->next(result)) 
  {
    STACK_PUSH(true, state);
  }

  STACK_END(state);
}


void IndexRangeProbeIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
  {
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


/*******************************************************************************
  IndexRangeProbeIterator
********************************************************************************/


IndexGeneralRangeProbeIterator::IndexGeneralRangeProbeIterator(
    static_context* sctx,
    const QueryLoc& loc,
    std::vector<PlanIter_t>& children)
  : 
  NaryBaseIterator<IndexGeneralRangeProbeIterator,
                   IndexRangeProbeIteratorState>(sctx, loc, children),
  theCheckKeyType(true)
{
}


IndexGeneralRangeProbeIterator::~IndexGeneralRangeProbeIterator() 
{
}


void IndexGeneralRangeProbeIterator::serialize(::zorba::serialization::Archiver& ar)
{
  serialize_baseclass(ar,
  (NaryBaseIterator<IndexGeneralRangeProbeIterator, IndexRangeProbeIteratorState>*)this);

  ar & theCheckKeyType;
}


bool IndexGeneralRangeProbeIterator::nextImpl(
    store::Item_t& result, 
    PlanState& aPlanState) const
{
  return false;
}


void IndexGeneralRangeProbeIterator::accept(PlanIterVisitor& v) const 
{
  v.beginVisit(*this);

  std::vector<PlanIter_t>::const_iterator lIter = theChildren.begin();
  std::vector<PlanIter_t>::const_iterator lEnd = theChildren.end();
  for ( ; lIter != lEnd; ++lIter )
  {
    (*lIter)->accept(v);
  }

  v.endVisit(*this);
}


}
