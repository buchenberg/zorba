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
#include <assert.h>

#include <zorba/stateless_function.h>

#include "context/static_context_consts.h"
#include "context/static_context.h"
#include "context/namespace_context.h"
#include "context/collation_cache.h"
#include "context/context_impl.h"

#include "compiler/expression/expr_base.h"

#include "zorbatypes/collation_manager.h"
#include "zorbatypes/Unicode_util.h"
#include "zorbatypes/URI.h"

#include "api/unmarshaller.h"

#include "zorbaerrors/error_manager.h"
#include "system/globalenv.h"

#include "types/typemanager.h"
#include "types/casting.h"

#include "functions/function.h"
#include "functions/library.h"
#include "functions/signature.h"

#include "store/api/store.h"
#include "store/api/item_factory.h"


// MS Visual Studio does not fully support throw(), and issues a warning
#ifndef _MSC_VER
#define THROW_XQP_EXCEPTION   throw(xqp_exception)
#else
#define THROW_XQP_EXCEPTION   
#endif

using namespace std;
namespace zorba {

#define ITEM_FACTORY (GENV.getStore().getItemFactory())

static_context::static_context()
  :
  context(NULL)
{
  set_encapsulating_entity_baseuri ("");
  set_entity_retrieval_url ("");
}

  
static_context::static_context (static_context *_parent)
  :
  context (_parent)
{
}


static_context::~static_context()
{
  //debug
  //test_obj.use_me();
  //end debug

  ///free the pointers from ctx_value_t from keymap
  checked_vector<hashmap<ctx_value_t>::entry>::const_iterator   it;
  const char    *keybuff;
  
  //keybuff[sizeof(keybuff)-1] = 0;
  for(it = keymap.begin();it!=keymap.end();it++)
  {
    ///it is an entry
    //keymap.getentryKey(*it, keybuff, sizeof(keybuff)-1);
    keybuff = (*it).key.c_str();
    const ctx_value_t *val = &(*it).val;

    if (0 == strncmp(keybuff, "type:", 5)) {
      RCHelper::removeReference (const_cast<XQType *> (val->typeValue));
    } else if (0 == strncmp(keybuff, "var:", 4)) {
      RCHelper::removeReference (const_cast<expr *> (val->exprValue));
    } else if (0 == strncmp(keybuff, "fn:", 3)) {
      RCHelper::removeReference (const_cast<function *> (val->functionValue));
    }
  }
}

void context::bind_expr (xqp_string key, expr *e) {
  ctx_value_t v = { e };
  RCHelper::addReference (e);
  keymap.put (key, v);
}

void context::bind_func (xqp_string key, function *f) {
  ctx_value_t v;
  if (lookup_func (key) != NULL)
    ZORBA_ERROR_PARAM (XQST0034, key, "");
  v.functionValue = f;
  RCHelper::addReference (f);
  keymap.put (key, v);
}



DECL_ENUM_PARAM (static_context, construction_mode)
DECL_ENUM_PARAM (static_context, order_empty_mode)
DECL_ENUM_PARAM (static_context, boundary_space_mode)
DECL_ENUM_PARAM (static_context, inherit_mode)
DECL_ENUM_PARAM (static_context, preserve_mode)
DECL_ENUM_PARAM (static_context, xpath1_0compatib_mode)
DECL_ENUM_PARAM (static_context, ordering_mode)

// DECL_STR_PARAM (static_context, baseuri)
// DECL_STR_PARAM (static_context, default_collation)
DECL_STR_PARAM (static_context, default_function_namespace, XQST0066)
DECL_STR_PARAM (static_context, default_elem_type_ns, XQST0066)
DECL_STR_PARAM (static_context, current_absolute_baseuri, MAX_ZORBA_ERROR_CODE)
DECL_STR_PARAM_TRIGGER (static_context, encapsulating_entity_baseuri, MAX_ZORBA_ERROR_CODE, set_current_absolute_baseuri (""))
DECL_STR_PARAM_TRIGGER (static_context, entity_retrieval_url, MAX_ZORBA_ERROR_CODE, set_current_absolute_baseuri (""))

TypeManager *static_context::get_typemanager ()
{
  TypeManager *tm = typemgr.get();
  if (tm != NULL) {
    return tm;
  }
  return dynamic_cast<static_context *>(parent)->get_typemanager();
}


void static_context::set_typemanager(std::auto_ptr<TypeManager> _typemgr)
{
  typemgr = _typemgr;
}


pair<xqp_string, xqp_string> parse_qname (xqp_string qname)
{
  std::string::size_type n = static_cast<std::string> (qname).find (':');
  return (n == string::npos)
    ? pair<xqp_string, xqp_string> ("", qname)
    : pair<xqp_string, xqp_string> (qname.substr (0, n), qname.substr (n+1));
}
  

xqp_string qname_internal_key2 (xqp_string ns, xqp_string local)
{
  return local + ":" + ns;
}


store::Item_t static_context::lookup_qname(
    xqp_string default_ns,
    xqp_string prefix,
    xqp_string local) const
{
  // Note: lookup_ns throws exception if there is no binding for the prefix.
  return ITEM_FACTORY->createQName((prefix.empty() ? default_ns.getStore() :
                                                     lookup_ns(prefix).getStore()),
                                   prefix.getStore(),
                                   local.getStore());
}


store::Item_t static_context::lookup_qname (xqp_string default_ns, xqp_string qname) const
{
  pair<xqp_string, xqp_string> rqname = parse_qname (qname);
  return lookup_qname (default_ns, rqname.first, rqname.second);
}


  xqp_string static_context::qname_internal_key (const store::Item *qname)
  {
    return qname_internal_key2 (qname->getNamespace (), qname->getLocalName ());
  }

  xqp_string static_context::qname_internal_key (xqp_string default_ns, xqp_string prefix, xqp_string local) const
  {
    return qname_internal_key2(prefix.empty () ? default_ns : lookup_ns (prefix),
                               local);
  }

  xqp_string static_context::qname_internal_key (xqp_string default_ns, xqp_string qname) const
  {
    pair<xqp_string, xqp_string> rqname = parse_qname (qname);
    return qname_internal_key (default_ns, rqname.first, rqname.second);
  }

  xqp_string static_context::fn_internal_key (int arity) 
  {
    return "fn:" + to_string (arity) + "/";
  }


function *static_context::lookup_fn (xqp_string prefix, xqp_string local, int arity) const 
{
  function *f = lookup_func (fn_internal_key (arity) +
                             qname_internal_key (default_function_namespace (),
                                                 prefix,
                                                 local));
  if (f != NULL)
    return f;
  else 
  {
    f = lookup_func (fn_internal_key (VARIADIC_SIG_SIZE) +
                     qname_internal_key (default_function_namespace (),
                                         prefix,
                                         local));
    if (f == NULL)
      ZORBA_ERROR_PARAM (XPST0017, (prefix.empty () ? prefix : (prefix + ":")) + local, to_string (arity));
    return f;
  }
}

  bool static_context::lookup_ns (xqp_string prefix, xqp_string &ns) const {
    return context_value ("ns:" + prefix, ns) && ! ns.empty();    
  }

  xqp_string static_context::lookup_ns (xqp_string prefix, const XQUERY_ERROR& err) const {
    xqp_string ns;
    if (! lookup_ns (prefix, ns)) {
      if (err != MAX_ZORBA_ERROR_CODE)
        ZORBA_ERROR_PARAM(err, prefix, "");
    }
    return ns;
  }

  xqp_string static_context::lookup_ns_or_default (xqp_string prefix, xqp_string default_ns) const {
    xqp_string ns;
    if (! lookup_ns (prefix, ns)) {
      return default_ns;
    }
    return ns;
  }

  bool static_context::lookup_elem_namespace(const xqp_string pfx, xqp_string& ns) const {
    if (pfx.empty())
    {
      ns = default_elem_type_ns(); 
    } else {
      if (!context_value("ns:" + pfx, ns))
      {
        return false;
      }
    }
    if (ns.empty())
      return false;
    else
      return true;
  }

  void static_context::bind_ns (xqp_string prefix, xqp_string ns, const XQUERY_ERROR& err)
  {
    bind_str ("ns:" + prefix, ns, err);
  }

  function *static_context::lookup_builtin_fn (xqp_string local, int arity)
  {
    function *f = GENV.getRootStaticContext().lookup_func (fn_internal_key (arity) + qname_internal_key2 (XQUERY_FN_NS, local));
    if (f == NULL)
      ZORBA_NOT_IMPLEMENTED ("built-in `" + local + "/" + to_string (arity) + "'");
    return f;
  }

  xqtref_t static_context::lookup_type( xqp_string key)
  {
    ctx_value_t val;
    ZORBA_ASSERT (context_value (key, val));
    return val.typeValue;
  }

  void  static_context::bind_type(xqp_string key, xqtref_t t)
  {
    ctx_value_t v;
    v.typeValue = &*t;
    keymap.put (key, v);
    RCHelper::addReference (const_cast<XQType *> (t.getp ()));
  }

  void static_context::add_variable_type(
    const xqp_string var_name, 
    xqtref_t var_type)
  {
    bind_type("type:var:" + qname_internal_key("", var_name), var_type);
  }

  xqtref_t  static_context::get_variable_type(
    store::Item *var_name)
  {
    return lookup_type( "type:var:" + qname_internal_key("",
                                                         var_name->getPrefix(),
                                                         var_name->getLocalName()));
  }

  void static_context::set_context_item_static_type(xqtref_t t)
  {
    bind_type("type:context:", t);
  }

  xqtref_t    static_context::context_item_static_type()
  {
    return lookup_type("type:context:");
  }

  void static_context::set_default_collection_type(xqtref_t t)
  {
    bind_type("type:defcollection:", t);
  }

  xqtref_t    static_context::default_collection_type()
  {
    return lookup_type("type:defcollection:");
  }

void static_context::set_function_type(const store::Item *qname, xqtref_t t)
{
  bind_type("type:fun:"+qname_internal_key( default_function_namespace(),
                                            qname->getPrefix(),
                                            qname->getLocalName()),
            t);
}

xqtref_t static_context::get_function_type(
  const store::Item_t qname) 
{
  return lookup_type("type:fun:" + qname_internal_key(default_function_namespace(),
                                                      qname->getPrefix(),
                                                      qname->getLocalName()));
}

void static_context::set_document_type(xqp_string docURI, xqtref_t t)
{
  bind_type("type:doc:"+docURI, t);
}

xqtref_t static_context::get_document_type(
  const xqp_string docURI) 
{
  return lookup_type("type:doc:" + docURI);

}

void static_context::set_collection_type(xqp_string collURI, xqtref_t t)
{
  bind_type("type:collection:"+collURI, t);
}

xqtref_t static_context::get_collection_type(
  const xqp_string collURI) 
{
  return lookup_type("type:collection:" + collURI);
}

/**
 * collation management
 */
void 
static_context::add_collation(const xqp_string& aURI)
{
  xqp_string lURI = resolve_relative_uri(aURI); 
  XQPCollator* lCollator = CollationFactory::createCollator(lURI);
  if (lCollator == 0)
  {
    ZORBA_ERROR_DESC( XQST0038, "invalid collation uri");
  }
  else
  {
    delete lCollator;
    bind_collation(lURI);
  }
}

CollationCache*
static_context::get_collation_cache() 
{
  return new CollationCache(this);
}

void
static_context::release_collation_cache(CollationCache* aCache)
{
  delete aCache;
}

XQPCollator*
static_context::create_collator(const xqp_string& aURI)
{
  return CollationFactory::createCollator(aURI); 
}

xqp_string 
static_context::default_collation_uri() const
{
  xqp_string lURI;
  if (!lookup_default_collation(lURI))
  {
    lURI = "http://www.flworfound.org/collations/IDENTICAL/en/US";
  }
  return lURI;
}

bool
static_context::has_collation_uri(const xqp_string& aURI) const
{
  return lookup_collation(aURI);
}

void 
static_context::set_default_collation_uri(const xqp_string& aURI)
{
  xqp_string lURI = resolve_relative_uri(aURI); 
  XQPCollator* lCollator = CollationFactory::createCollator(lURI);
  if (lCollator == 0)
  {
    ZORBA_ERROR_DESC_OSS( XQST0038, "invalid collation uri " << lURI);
  }
  else
  {
    delete lCollator;
    bind_default_collation(lURI);
  }
}


xqp_string static_context::baseuri () const 
{
  xqp_string val;                                        
  if(!context_value ("int:" "from_prolog_baseuri", val))  // if not found val remains ""
  {
    context_value("int:" "baseuri", val);
  }
  return val;
}

void static_context::set_baseuri (xqp_string val, bool from_prolog) 
{
  if (from_prolog)
    // throw XQST0032 if from_prolog_baseuri is already defined
    bind_str ("int:" "from_prolog_baseuri", val, XQST0032);
  else
    // overwite existing value of baseuri, if any
    str_keymap.put ("int:" "baseuri", val);

  compute_current_absolute_baseuri ();
}

void static_context::compute_current_absolute_baseuri()
{
  //if base Uri is present, compute absolute base Uri
  //else if encapsulating_entity_baseuri is present, use that
  //else if entity_retrieval_url is present, use that
  //else do not set the absolute baseuri (and hope there are no relative uris)

  xqp_string    prolog_baseuri;
  xqp_string    ee_baseuri;
  xqp_string    loaded_uri;

  prolog_baseuri = baseuri();

  if (!prolog_baseuri.empty() &&
      URI::is_valid (prolog_baseuri.getStore (), false)) {
    // is already absolute baseuri
    set_current_absolute_baseuri(prolog_baseuri);
    return;
  }
  if (!prolog_baseuri.empty()) {
    /// is relative, needs to be resolved
    ee_baseuri = encapsulating_entity_baseuri();
    if(!ee_baseuri.empty()) {
      set_current_absolute_baseuri(make_absolute_uri(prolog_baseuri, ee_baseuri));
      return;
    }
    loaded_uri = entity_retrieval_url();
    if(!loaded_uri.empty()) {
      set_current_absolute_baseuri(make_absolute_uri(prolog_baseuri, loaded_uri));
      return;
    }
    
    set_current_absolute_baseuri (make_absolute_uri(prolog_baseuri, implementation_baseuri()));
    return;
  }

  ee_baseuri = encapsulating_entity_baseuri();
  if(!ee_baseuri.empty()) {
    set_current_absolute_baseuri(ee_baseuri);
    return;
  }
  loaded_uri = entity_retrieval_url();
  if(!loaded_uri.empty()) {
    set_current_absolute_baseuri(loaded_uri);
    return;
  }
  set_current_absolute_baseuri (implementation_baseuri());
  return;
}

xqp_string static_context::make_absolute_uri(xqp_string uri, xqp_string base_uri) {
  xqpStringStore_t result;
  URI::error_t err = URI::resolve_relative (base_uri.getStore (),
                                            uri.getStore (),
                                            result);
  switch (err) {
  case URI::MAX_ERROR_CODE:
    return result.getp();
  default:
    ZORBA_ERROR (XQST0046);
  }
}

xqp_string static_context::final_baseuri () {
  // cached value
  string abs_base_uri = current_absolute_baseuri();

  if(abs_base_uri.empty()) {
    compute_current_absolute_baseuri();
    abs_base_uri = current_absolute_baseuri();
  }

  // won't happen -- we default to a non-empty URI
  if(abs_base_uri.empty()) {
    ZORBA_ERROR_DESC( XPST0001, "empty base URI");
    return "";
  }
  
  return abs_base_uri;
}

xqp_string static_context::resolve_relative_uri (xqp_string uri, xqp_string abs_base_uri) {
  return make_absolute_uri (uri, abs_base_uri.empty () ? final_baseuri () : abs_base_uri);
}

void static_context::import_module (const static_context *module) {
  checked_vector<hashmap<ctx_value_t>::entry>::const_iterator   it;
  const char    *keybuff;
  
  for(it = module->keymap.begin();it!=module->keymap.end();it++) {
    keybuff = (*it).key.c_str();
    const ctx_value_t *val = &(*it).val;

    if (0 == strncmp(keybuff, "var:", 4) && 0 != strncmp(keybuff, "var:$$", 6)) {
      bind_expr (keybuff, val->exprValue);
    } else if (0 == strncmp(keybuff, "fn:", 3)) {
      bind_func (keybuff, val->functionValue);
    }
  }
}


bool
static_context::bind_stateless_external_function(StatelessExternalFunction* aExternalFunction) {
  xqpString lLocalName = Unmarshaller::getInternalString(aExternalFunction->getLocalName());
  xqpString lURI = Unmarshaller::getInternalString(aExternalFunction->getURI());

  return bind_stateless_function(lLocalName +":" +lURI, aExternalFunction);
}

StatelessExternalFunction *
static_context::lookup_stateless_external_function(xqp_string aPrefix, xqp_string aLocalName)
{
  return lookup_stateless_function( 
    qname_internal_key(default_function_namespace(), aPrefix, aLocalName)); 
}

} /* namespace zorba */

