/*
 *  Copyright 2006-2007 FLWOR Foundation.
 *  Author: Tim Kraska
 *
 */


#ifndef XQP_RUNTIME_FLWOR_ITERATOR
#define XQP_RUNTIME_FLWOR_ITERATOR

#include "context/common.h"
#include "util/rchandle.h"
#include "util/tracer.h"
#include "compiler/parser/location.hh"
#include "runtime/base/iterator.h"
#include "runtime/base/unarybase.h"

#include <assert.h>
#include <iostream>
#include "util/checked_vector.h"

namespace xqp
{

class ForVarIterator;
typedef rchandle<ForVarIterator> var_iter_t;

class LetVarIterator;
typedef rchandle<LetVarIterator> ref_iter_t;
  
  
 
 /**
  Main FLWOR class designed after   http://www.w3.org/TR/xquery/#id-flwor-expressions. 
  The complete tuple-stream handling is done in this
  class. 
 */
class FLWORIterator : public Batcher<FLWORIterator>
{
public:
  class OrderKeyCmp;
      
  /**
     Wrappes a FOR or LET clause. 
     It is combined to avoid dynamic casts during the runtime
  */
  class ForLetClause 
  {
    friend class FLWORIterator;
    friend class PrinterVisitor;

  protected:
    enum ForLetType {FOR, LET};

    ForLetType               type;
    std::vector<var_iter_t>  forVars;
    std::vector<var_iter_t>  posVars;
    std::vector<ref_iter_t>  letVars;
    PlanIter_t               input;
    bool                     needsMaterialization;

  public:
    /**
     * Creates a new ForClause
     */
    ForLetClause(
        std::vector<var_iter_t> forVars,
        PlanIter_t& input);
          
    /**
     * Creates a new ForClause including positional variable bindings 
     */
    ForLetClause(
        std::vector<var_iter_t> forVars,
        std::vector<var_iter_t> posVars,
        PlanIter_t& input);

    /**
     * Creates a new LetClause
     * needsMaterialization indicates if it is necassary to materialize the LET-Binding:
     * E.g. "let $x := (1,2,3) return ($x, $x)" needs materialization.
     * but "let $x := (1,2,3) return if(test()) then $x else $x" doesn't
     */
    ForLetClause(
        std::vector<ref_iter_t> letVars,
        PlanIter_t& input,
        bool needsMaterialization);
          
    void accept (PlanIterVisitor&) const;
  };

  /**
   *  Wrapper for a OrderSpec
   * http://www.w3.org/TR/xquery/#id-orderby-return
   */
  class OrderSpec
  {
    friend class FLWORIterator;
    friend class OrderKeyCmp;
  protected:
    PlanIter_t orderByIter;
    bool empty_least;
    bool descending;
  public:
    void accept ( PlanIterVisitor& ) const;
    OrderSpec ( PlanIter_t orderByIter, bool empty_least, bool descending );
  };

  /**
   * Wrapper for a orderByClause
   * See http://www.w3.org/TR/xquery/#id-orderby-return
   */
  class OrderByClause
  {
    friend class FLWORIterator;
  public:
    std::vector<OrderSpec> orderSpecs;
    bool stable;
  public:
    void accept ( PlanIterVisitor& ) const;
    OrderByClause ( std::vector<OrderSpec> orderSpecs, bool stable );
  };


  /**
   * Class to pass to the MultiMap to do the comparison according to the OrderByClause. 
   * Luckily the MultiMap is stable already :-)
   */
  class OrderKeyCmp
  {
  public:
    OrderKeyCmp() : mOrderSpecs ( 0 ) {}
    OrderKeyCmp ( vector<OrderSpec>* aOrderSpecs ) : mOrderSpecs ( aOrderSpecs ) {}

    /**
     * The key comparison function, a Strict Weak Ordering whose argument type is key_type;
     * it returns true if its first argument is less than its second argument, and false otherwise.
     * This is also defined as multimap::key_compare.
     */
    bool operator() ( const std::vector<Item_t>& s1, const std::vector<Item_t>& s2 ) const;
          
    /**
     * Does the actual comparision
     * @return    -1, if item0 &lt; item1
     *            0, if item0 == item1
     *            1, if item0 &gt; item1
     */
    inline int8_t compare(
        const Item_t& s1,
        const Item_t& s2,
        bool asc,
        bool emptyLeast) const;
          
    /**
     * Helper functions to switch the ordering between ascending and descending
     */
    inline int8_t descAsc(int8_t result, bool asc) const;
    
  private:
    vector<OrderSpec>* mOrderSpecs; //Pointer to the OrderSpec to do the comparison accordingly  
  };
      
protected:
    
  typedef std::multimap<std::vector<Item_t>, Iterator_t, OrderKeyCmp> order_map_t;

  class FlworState : public PlanIteratorState
  {
  public:
    FlworState() ;
          
  public:
    //varBindingState holds if a LET is already bound or not and futhermore for FORs it holds
    //as wenn the positional integer value
    checked_vector<uint32_t> varBindingState;
          
    //orderMap, curOrderPos and curOrderResultSeq are just needed if we have a Orderclause
    //The MultiMap does the actual ordering
    //When returning the result, this indicates, which return sequence we are
    // touching at the moment and the curOrderPos indicates 
    order_map_t* orderMap; 
    Iterator_t curOrderResultSeq; 
    order_map_t::const_iterator curOrderPos; 
     
    /**
     * Init the state for a certain nb of variables but not the ordering
     * @nb_variables  Number of FOR and LET clauses
     */
    void init(size_t nb_variables);
          
    /**
     * Init the state for a certain nb of variables and ordering
     * @nb_variables  Number of FOR and LET clauses
     * @orderSpecs    The OrderSpec which defines how to compare during ordering
     */
    void init(size_t nb_variables, std::vector<OrderSpec>* orderSpecs);
    /**
     * Resets the state
     */
    void reset();
          
    /**
     * Release the resources (especially those we used for Ordering)
     */
    void releaseResources();
  };
  
  /* ####################################################
   * Here we have the actual FLWOR class
   * #################################################### 
   */
    
 public:
  /**
   * Constructor
   * @param loc location
   * @param forLetClauses For and Lets: Attention the order matters!
   * @param whereClause The where-clause iterator. Can be null
   * @param orderByClause The order by expressions. Can be null
   * @param returnClause The return expressions
   * @param whereClauseReturnsBooleanPlus Optional flag.
   *  If true => The iterator has to return xs:boolean+
   */
  FLWORIterator(
        const yy::location&         loc,
        std::vector<ForLetClause>&  forLetClauses,
        PlanIter_t&                 whereClause,
        OrderByClause*              orderByClause,
        PlanIter_t&                 returnClause,
        bool                        whereClauseReturnsBooleanPlus = false );
    
  ~FLWORIterator();

  Item_t nextImpl(PlanState& planState);
  void resetImpl(PlanState& planState);
  void releaseResourcesImpl(PlanState& planState);

  virtual uint32_t getStateSize() const  { return sizeof ( FlworState); }
  virtual uint32_t getStateSizeOfSubtree() const;
  virtual void setOffset ( PlanState& planState, uint32_t& offset );
  
  void accept ( PlanIterVisitor& ) const;

 private:
  /**
   * Resets a input of a FOR or LET
   */
  void resetInput(const int& varNb, FlworState* flworState, PlanState& planState);
      
  /**
   * Binds the variable to all its places
   */
  bool bindVariable ( int varNb, FlworState* flworState, PlanState& planState );
    
  /**
   * Evaluates the where clause. If there doesn't exist a where clause it returns always true
   */
  bool evalWhereClause( PlanState& planState );
    
  /**
   * Materialized the result after binding the variables (needed for OrderBy)
   */
  void matResultAndOrder (FlworState* flworState, PlanState& planState);
      
 private:
  std::vector<ForLetClause> forLetClauses;
  PlanIter_t                whereClause; //can be null
  OrderByClause           * orderByClause;  //can be null
  bool                      doOrderBy; //just indicates if the FLWOR has an orderby
  PlanIter_t                returnClause; 
  bool                      whereClauseReturnsBooleanPlus;
  const int                 bindingsNb; //Number of FORs and LETs (overall)  
};


} /* namespace xqp */
#endif  /* XQP_ITEM_ITERATOR_H */
