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

#ifndef ZORBA_STORE_ATOMIC_ITEMS_H
#define ZORBA_STORE_ATOMIC_ITEMS_H

#include <zorba/config.h>
#include <vector>

#include "store/api/item.h"
#include "store/naive/store_defs.h"
#ifndef ZORBA_NO_FULL_TEXT
#include "store/naive/naive_ft_token_iterator.h"
#endif /* ZORBA_NO_FULL_TEXT */

#include "zorbatypes/representations.h"
#include "zorbatypes/datetime.h"
#include "zorbautils/tokenizer.h"

#include "zorbaerrors/errors.h"


namespace zorba
{

namespace simplestore
{

class AtomicItemTokenizer;
class QNameItem;
typedef rchandle<QNameItem> QNameItem_t;


/*****************************************************************************

 *****************************************************************************/

class AtomicItem : public store::Item
{
#ifdef ZORBA_FOR_ONE_THREAD_ONLY
public:
  AtomicItem() : store::Item() { }

#else

protected:
  SYNC_CODE(RCLock  theRCLock;)

public:
  AtomicItem() : store::Item() { SYNC_CODE(theRCLockPtr = &theRCLock;) }
#endif

  ~AtomicItem() {}

  bool isNode() const     { return false; }
  bool isAtomic() const   { return true;  }
  bool isList() const     { return false; }
  bool isPul() const      { return false; }
  bool isTuple() const    { return false; }
  bool isError() const    { return false; }
  bool isFunction() const { return false; }

  store::Item_t getAtomizationValue() const;

  virtual SchemaTypeCode getTypeCode() const = 0;

  void getTypedValue(store::Item_t& val, store::Iterator_t& iter) const;

  bool castToLong(store::Item_t& result) const;

  void coerceToDouble(store::Item_t& result, bool force, bool& lossy) const;
};


typedef rchandle<AtomicItem> AtomicItem_t;


/*******************************************************************************
  class UserTypedAtomicItem
********************************************************************************/
class UserTypedAtomicItemImpl : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  AtomicItem_t   theBaseItem;
  store::Item_t  theTypeName;

protected:
  UserTypedAtomicItemImpl(
        store::Item_t& baseItem,
        store::Item_t& typeName)
  {
    theBaseItem.transfer(baseItem);
    theTypeName.transfer(typeName);
  }

  UserTypedAtomicItemImpl() {}

public:
  store::Item* getBaseItem() const { return theBaseItem.getp(); }

  SchemaTypeCode getTypeCode() const { return theBaseItem->getTypeCode(); }

  store::Item* getType() const { return theTypeName.getp(); }

  uint32_t hash(long timezone = 0, const XQPCollator* collation = 0) const
  {
    return theBaseItem->hash(timezone, collation);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    if (other->getBaseItem() == NULL)
      return theBaseItem->equals(other, timezone, collation);
    else
      return theBaseItem->equals(other->getBaseItem(), timezone, collation);
  }

  long compare(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    if (other->getBaseItem() == NULL)
      return theBaseItem->compare(other, timezone, collation);
    else
      return theBaseItem->compare(other->getBaseItem(), timezone, collation);
  }

  store::Item_t getEBV() const { return theBaseItem->getEBV(); }

  zstring getStringValue() const { return theBaseItem->getStringValue(); }

  void getStringValue2(zstring& val) const { theBaseItem->getStringValue2(val); }

  void appendStringValue(zstring& buf) const { theBaseItem->appendStringValue(buf); }

  const zstring& getString() const { return theBaseItem->getString(); }

  xs_base64Binary getBase64BinaryValue() const { return theBaseItem->getBase64BinaryValue(); }

  xs_hexBinary getHexBinaryValue() const { return theBaseItem->getHexBinaryValue(); }

  bool getBooleanValue() const { return theBaseItem->getBooleanValue(); }

  xs_double getDoubleValue() const { return theBaseItem->getDoubleValue(); }

  xs_float getFloatValue() const { return theBaseItem->getFloatValue(); }

  xs_decimal getDecimalValue() const { return theBaseItem->getDecimalValue(); }

  xs_integer getIntegerValue() const { return theBaseItem->getIntegerValue(); }

  xs_uinteger getUnsignedIntegerValue() const { return theBaseItem->getUnsignedIntegerValue(); }

  xs_long getLongValue() const { return theBaseItem->getLongValue(); }

  xs_int getIntValue() const { return theBaseItem->getIntValue(); }

  xs_short getShortValue() const { return theBaseItem->getShortValue(); }

  xs_byte getByteValue() const { return theBaseItem->getByteValue(); }

  xs_ulong getUnsignedLongValue() const { return theBaseItem->getUnsignedLongValue(); }

  xs_uint getUnsignedIntValue() const { return theBaseItem->getUnsignedIntValue(); }

  xs_ushort getUnsignedShortValue() const { return theBaseItem->getUnsignedShortValue(); }

  xs_ubyte getUnsignedByteValue() const { return theBaseItem->getUnsignedByteValue(); }

  bool isNaN() const { return theBaseItem->isNaN(); }

  bool isPosOrNegInf() const { return theBaseItem->isPosOrNegInf(); }

  const xs_dateTime& getDateTimeValue() const
  {
    return theBaseItem->getDateTimeValue();
  }

  const xs_date& getDateValue() const
  {
    return theBaseItem->getDateValue();
  }

  const xs_time& getTimeValue() const
  {
    return theBaseItem->getTimeValue();
  }

  const xs_gYearMonth& getGYearMonthValue() const
  {
    return theBaseItem->getGYearMonthValue();
  }

  const xs_gYear& getGYearValue() const
  {
    return theBaseItem->getGYearValue();
  }

  const xs_gMonth& getGMonthValue() const
  {
    return theBaseItem->getGMonthValue();
  }

  const xs_gMonthDay& getGMonthDayValue() const
  {
    return theBaseItem->getGMonthDayValue();
  }

  const xs_gDay& getGDayValue() const
  {
    return theBaseItem->getGDayValue();
  }

  const xs_duration& getDurationValue() const
  {
    return theBaseItem->getDurationValue();
  }

  const xs_dayTimeDuration& getDayTimeDurationValue() const
  {
    return theBaseItem->getDayTimeDurationValue();
  }

  const xs_yearMonthDuration& getYearMonthDurationValue() const
  {
    return theBaseItem->getYearMonthDurationValue();
  }

  zstring show() const { return "UserTypedAtomicItem [" + theBaseItem->show() + "]"; }
};


/*******************************************************************************
  class UntypedAtomicItem
********************************************************************************/
class UntypedAtomicItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  zstring theValue;

protected:
  UntypedAtomicItem(zstring& value) { theValue.take(value); }

  UntypedAtomicItem() {}

public:
  bool castToUri(store::Item_t& result) const;

  bool castToString(store::Item_t& result) const;

  bool castToDateTime(store::Item_t& result) const;

  bool castToDate(store::Item_t& result) const;

  bool castToTime(store::Item_t& result) const;

  bool castToGYear(store::Item_t& result) const;

  bool castToGYearMonth(store::Item_t& result) const;

  bool castToGMonthDay(store::Item_t& result) const;

  bool castToGMonth(store::Item_t& result) const;

  bool castToGDay(store::Item_t& result) const;

  bool castToDuration(store::Item_t& result) const;

  bool castToDouble(store::Item_t& result) const;

  bool castToDecimal(store::Item_t& result) const;

  bool castToInteger(store::Item_t& result) const;

  bool castToHexBinary(store::Item_t& result) const;

  bool castToBase64Binary(store::Item_t& result) const;

  bool castToBoolean(store::Item_t& result) const;

  SchemaTypeCode getTypeCode() const { return XS_UNTYPED_ATOMIC; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue == other->getString();
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const { return theValue; }

  void getStringValue2(zstring& val) const { val = theValue; }

  void appendStringValue(zstring& buf) const { buf += theValue; }

  const zstring& getString() const { return theValue; }

  zstring show() const;
};


/*******************************************************************************
  class QNameItem
********************************************************************************/
class QNameItem : public AtomicItem
{
  friend class QNamePool;

protected:

  zstring      theNamespace;
  zstring      thePrefix;
  zstring      theLocal;

  QNameItem_t  theNormQName;

  uint16_t     thePosition;
  uint16_t     theNextFree;
  uint16_t     thePrevFree;

public:
  ~QNameItem();

  QNameItem* getNormalized() const
  {
    return (isNormalized() ? const_cast<QNameItem*>(this) : theNormQName.getp());
  }

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* item,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return (getNormalized() == static_cast<const QNameItem*>(item)->getNormalized());
  }

  const zstring& getNamespace() const { return theNamespace; }

  const zstring& getPrefix() const { return thePrefix; }

  const zstring& getLocalName() const { return getNormalized()->theLocal; }

  SchemaTypeCode getTypeCode() const { return XS_QNAME; }

  store::Item* getType() const;

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  bool isIdQName() const;

  bool isBaseUri() const;

  zstring show() const;

protected:
  QNameItem() : thePosition(0), theNextFree(0), thePrevFree(0)
  {
  }

  void free();

  bool isValid() const { return !theLocal.empty() || theNormQName != NULL; }

  bool isInCache() const { return thePosition != 0; }

  bool isOverflow() const { return thePosition == 0; }

  bool isNormalized() const { return thePrefix.empty(); }

  void setNormQName(QNameItem* qn)
  {
    assert(theLocal.empty() && theNormQName == NULL && !isNormalized());

    theNormQName = qn;
  }

  void unsetNormQName()
  {
    assert(theLocal.empty() && theNormQName != NULL && !isNormalized());

    theNormQName = NULL;
  }

  QNameItem* detachNormQName()
  {
    assert(theLocal.empty() && theNormQName != NULL && !isNormalized());

    return theNormQName.release();
  }

  void setLocalName(const zstring& local)
  {
    assert(theLocal.empty() && theNormQName == NULL && isNormalized());

    theLocal = local;
  }

  void unsetLocalName()
  {
    assert(!theLocal.empty() && theNormQName == NULL && isNormalized());

    theLocal.clear();
  }
};


/*******************************************************************************
  class NotationItem
********************************************************************************/
class NotationItem : public AtomicItem
{
protected:
  QNameItem_t       theQName;

protected:
  friend class BasicItemFactory;
  NotationItem(const zstring& nameSpace,
               const zstring& prefix,
               const zstring& localName);

  NotationItem(store::Item* qname);

public:
  virtual ~NotationItem();

  store::Item* getType() const;

  bool equals(const store::Item* item,
              long timezone = 0,
              const XQPCollator* aCollation = 0) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;

  virtual SchemaTypeCode getTypeCode() const { return XS_NOTATION; }
};


/*******************************************************************************
  class AnyUriItem
********************************************************************************/
class AnyUriItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  zstring theValue;

protected:
  AnyUriItem(zstring& value) { theValue.take(value); }

  AnyUriItem() {}

public:
  SchemaTypeCode getTypeCode() const { return XS_ANY_URI; }

  store::Item* getType( ) const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* item,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return item->getString() == theValue;
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue.compare(other->getString());
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const { return theValue; }

  void getStringValue2(zstring& val) const { val = theValue; }

  void appendStringValue(zstring& buf) const { buf += theValue; }

  const zstring& getString() const { return theValue; }

  zstring show() const;
};


/******************************************************************************
  class StringItem
 *****************************************************************************/

class StringItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItemTokenizer;

protected:
  zstring theValue;

protected:
  StringItem(zstring& value) { theValue.take(value); }

  StringItem() {}

public:

  virtual SchemaTypeCode getTypeCode() const { return XS_STRING; }

  virtual store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item*,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const;

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const;

  store::Item_t getEBV() const;

  zstring getStringValue() const { return theValue; }

  void getStringValue2(zstring& val) const { val = theValue; }

  void appendStringValue(zstring& buf) const { buf += theValue; }

  const zstring& getString() const { return theValue; }

  virtual zstring show() const;

#ifndef ZORBA_NO_FULL_TEXT
  FTTokenIterator_t
  getDocumentTokens( locale::iso639_1::type = locale::iso639_1::unknown ) const;

  FTTokenIterator_t
  getQueryTokens( locale::iso639_1::type = locale::iso639_1::unknown,
                  bool wildcards = false ) const;
#endif /* ZORBA_NO_FULL_TEXT */
};


#ifndef ZORBA_NO_FULL_TEXT
/**
 * An <code>AtomicItemTokenizer</code> is-a Tokenizer::Callback TODO
 */
class AtomicItemTokenizer : public Tokenizer::Callback
{
public:
  typedef NaiveFTTokenIterator::FTTokens FTTokens;

  AtomicItemTokenizer( Tokenizer &tokenizer, locale::iso639_1::type lang,
                       FTTokens &tokens ) :
    tokenizer_( tokenizer ),
    tokens_( tokens ),
    lang_( lang )
  {
  }

  void operator()( char const *utf8_s, int utf8_len,
                   int token_no, int sent_no, int para_no, void* = 0 );

  void tokenize( char const *utf8_s, int len ) {
    tokenizer_.tokenize( utf8_s, len, lang_, *this );
  }

private:
  Tokenizer &tokenizer_;
  FTTokens &tokens_;
  locale::iso639_1::type const lang_;
};
#endif /* ZORBA_NO_FULL_TEXT */


/*******************************************************************************
  class NormalizedStringItemImpl
********************************************************************************/
class NormalizedStringItemImpl : public StringItem
{
  friend class BasicItemFactory;

protected:
  NormalizedStringItemImpl(zstring& value) : StringItem(value) {}

public:
  virtual SchemaTypeCode getTypeCode() const { return XS_NORMALIZED_STRING; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class TokenItemImpl
********************************************************************************/
class TokenItemImpl : public NormalizedStringItemImpl
{
  friend class BasicItemFactory;

public:
  TokenItemImpl(zstring& value) : NormalizedStringItemImpl(value) {}

  virtual SchemaTypeCode getTypeCode() const { return XS_TOKEN; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class NMTOKENItemImpl
********************************************************************************/
class NMTOKENItemImpl : public TokenItemImpl
{
  friend class BasicItemFactory;

protected:
  NMTOKENItemImpl(zstring& value) : TokenItemImpl(value) {}

public:
  virtual SchemaTypeCode getTypeCode() const { return XS_NMTOKEN; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class LanguageItem
********************************************************************************/
class LanguageItemImpl : public TokenItemImpl
{
  friend class BasicItemFactory;

protected:
  LanguageItemImpl(zstring& value) : TokenItemImpl(value) {}

public:
  virtual SchemaTypeCode getTypeCode() const { return XS_LANGUAGE; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class NameItem
********************************************************************************/
class NameItemImpl : public TokenItemImpl
{
  friend class BasicItemFactory;

protected:
  NameItemImpl(zstring& value) : TokenItemImpl(value) {}

public:
  virtual SchemaTypeCode getTypeCode() const { return XS_NAME; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class NCNameItem
********************************************************************************/
class NCNameItemImpl : public NameItemImpl
{
  friend class BasicItemFactory;

protected:
  NCNameItemImpl(zstring& value) : NameItemImpl(value) {}

public:
  virtual SchemaTypeCode getTypeCode() const { return XS_NCNAME; }

  virtual store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class IDItem
********************************************************************************/
class IDItemImpl : public NCNameItemImpl
{
  friend class BasicItemFactory;

protected:
  IDItemImpl(zstring& value) : NCNameItemImpl(value) { }

public:
  SchemaTypeCode getTypeCode() const { return XS_ID; }

  store::Item* getType() const;

  virtual zstring show() const;
};

/*******************************************************************************
  class IDREFItem
********************************************************************************/
class IDREFItemImpl : public NCNameItemImpl
{
  friend class BasicItemFactory;

protected:
  IDREFItemImpl(zstring& value) : NCNameItemImpl(value) { }

public:
  SchemaTypeCode getTypeCode() const { return XS_IDREF; }

  store::Item* getType() const;

  virtual zstring show() const;
};

/*******************************************************************************
  class ENTITYItemImpl
********************************************************************************/
class ENTITYItemImpl : public NCNameItemImpl
{
  friend class BasicItemFactory;

protected:
  ENTITYItemImpl(zstring& value) : NCNameItemImpl(value) { }

public:
  SchemaTypeCode getTypeCode() const { return XS_ENTITY; }

  store::Item* getType() const;

  virtual zstring show() const;
};


/*******************************************************************************
  class DateTimeItem
********************************************************************************/
class DateTimeItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  DateTime theValue;

protected:
  DateTimeItem(const DateTime* aValue) : theValue(*aValue) { };

  DateTimeItem() { };

public:
  const xs_dateTime& getDateTimeValue() const { return theValue; }

  const xs_date& getDateValue() const { return theValue; }

  const xs_time& getTimeValue() const { return theValue; }

  const xs_gYearMonth& getGYearMonthValue() const { return theValue; }

  const xs_gYear& getGYearValue() const { return theValue; }

  const xs_gMonth& getGMonthValue() const { return theValue; }

  const xs_gMonthDay& getGMonthDayValue() const { return theValue; }

  const xs_gDay& getGDayValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const;

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item*,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const;

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const;

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class DurationItem
********************************************************************************/
class DurationItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  Duration theValue;

protected:
  DurationItem(const xs_duration* aValue) : theValue(*aValue) { };

  DurationItem() {}

public:
  const xs_duration& getDurationValue() const;

  const xs_dayTimeDuration& getDayTimeDurationValue() const;

  const xs_yearMonthDuration& getYearMonthDurationValue() const;

  SchemaTypeCode getTypeCode() const;

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return theValue.hash();
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue == other->getDurationValue();
  }


  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue.compare(other->getDurationValue());
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class DoubleItem
********************************************************************************/
class DoubleItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
	xs_double theValue;

protected:
	DoubleItem(const xs_double& aValue) : theValue( aValue ) {}

  DoubleItem() {}

public:
	xs_double getDoubleValue() const { return theValue; }

  bool isNaN() const;
  bool isPosOrNegInf() const;

  SchemaTypeCode getTypeCode() const { return XS_DOUBLE; }

	store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue == other->getDoubleValue();
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    if (theValue.isNaN() || other->getDoubleValue().isNaN())
      return 2;

    return theValue.compare(other->getDoubleValue());
  }

	store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class FloatItem
********************************************************************************/
class FloatItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  xs_float theValue;

protected:
  FloatItem(const xs_float& aValue) : theValue ( aValue ) {}

  FloatItem() {}

public:
  xs_float getFloatValue() const { return theValue; }

  xs_double getDoubleValue() const { return FloatCommons::parseFloat(theValue); }

  bool isNaN() const;

  bool isPosOrNegInf() const;

  SchemaTypeCode getTypeCode() const { return XS_FLOAT; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return getDoubleValue() == other->getDoubleValue();
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    if (theValue.isNaN() || other->getFloatValue().isNaN())
      return 2;

    return getDoubleValue().compare(other->getDoubleValue());
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class DecimalItem

                               decimal
                                  |
                                  |
                               integer
                                  |
              ------------------------------------------
              |                   |                    |
       nonPositiveInteger  nonNegativeInteger         long
              |                   |                    |
              |            -----------------          int
        negativeInteger    |               |           |
                           |               |         short
                     positiveInteger   unsignedLong    |
                                           |         byte
                                       unsignedInt
                                           |
                                       unsingedShort
                                           |
                                       unsingedByte

********************************************************************************/
class DecimalItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class IndexBoxCondition;
  friend class AtomicItem;

protected:
  xs_decimal theValue;

  DecimalItem(const xs_decimal& aValue) : theValue(aValue) {}

  DecimalItem() {}

public:
  xs_decimal getDecimalValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_DECIMAL; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return theValue.hash();
  }

  bool equals(
        const store::Item* other,
        long timezone = 0 ,
        const XQPCollator* aCollation = 0) const
  {
    return theValue == other->getDecimalValue();
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return theValue.compare(other->getDecimalValue());
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  bool isNaN() const;

  zstring show() const;
};


/*******************************************************************************
  class IntegerItem
********************************************************************************/
class IntegerItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_integer theValue;

protected:
  IntegerItem(const xs_integer& aValue) : theValue ( aValue ) {}

  IntegerItem() {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const { return theValue; }

  xs_long getLongValue() const;

  virtual SchemaTypeCode getTypeCode() const { return XS_INTEGER; }

  virtual store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return theValue.hash();
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    try
    {
      return theValue == other->getIntegerValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    try
    {
      return theValue.compare(other->getIntegerValue());
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  bool isNaN() const { return false; }

  virtual zstring show() const;
};


/*******************************************************************************
  class NonPositiveIntegerItem
********************************************************************************/
class NonPositiveIntegerItem : public IntegerItem
{
  friend class BasicItemFactory;

protected:
  NonPositiveIntegerItem(const xs_integer& aValue) : IntegerItem(aValue) {}

  NonPositiveIntegerItem() {}

public:
  SchemaTypeCode getTypeCode() const { return XS_NON_POSITIVE_INTEGER; }

  store::Item* getType() const;

  zstring show() const;
};


/*******************************************************************************
  class NegativeIntegerItem
********************************************************************************/
class NegativeIntegerItem : public IntegerItem
{
  friend class BasicItemFactory;

protected:
  NegativeIntegerItem(const xs_integer& aValue) : IntegerItem(aValue) {}

  NegativeIntegerItem() {}

public:
  SchemaTypeCode getTypeCode() const { return XS_NEGATIVE_INTEGER; }

  store::Item* getType() const;

  zstring show() const;
};


/*******************************************************************************
  class NonNegativeIntegerItem

  Note: xs_uinteger is typedef of Integer
********************************************************************************/
class NonNegativeIntegerItem : public IntegerItem
{
  friend class BasicItemFactory;

protected:
  NonNegativeIntegerItem(const xs_uinteger& aValue) : IntegerItem(aValue) {}

  NonNegativeIntegerItem() {}

public:
  xs_uinteger getUnsignedIntegerValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_NON_NEGATIVE_INTEGER; }

  store::Item* getType() const;

  zstring show() const;
};


/*******************************************************************************
  class PositiveIntegerItem
********************************************************************************/
class PositiveIntegerItem : public  IntegerItem
{
  friend class BasicItemFactory;

protected:
  PositiveIntegerItem(const xs_uinteger& aValue) : IntegerItem(aValue) { }

  PositiveIntegerItem() {}

public:
  xs_uinteger getUnsignedIntegerValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_POSITIVE_INTEGER; }

  store::Item* getType() const;

  zstring show() const;
};


/*******************************************************************************
  class LongItem
********************************************************************************/
class LongItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_long theValue;

protected:
  LongItem(xs_long aValue) : theValue(aValue) {}

  LongItem() {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_long getLongValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_LONG; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getLongValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    try
    {
      return (theValue < other->getLongValue() ?
              -1 :
              (theValue == other->getLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  bool isNaN() const { return false; }

  zstring show() const;
};


/*******************************************************************************
  class IntItem
********************************************************************************/
class IntItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  int32_t theValue;

protected:
  IntItem(xs_int aValue) : theValue(aValue) {}

  IntItem() {}

public:
  bool isNaN() const { return false; }

  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  int32_t getIntValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_INT; }

  store::Item* getType( ) const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getLongValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    try
    {
      return (theValue < other->getLongValue() ?
              -1 :
              (theValue == other->getLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class ShortItem
********************************************************************************/
class ShortItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_short theValue;

protected:
  ShortItem(xs_short aValue)  : theValue(aValue) {}

  ShortItem() {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  xs_int getIntValue() const { return static_cast<xs_int>(theValue); }

  xs_short getShortValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_SHORT; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getLongValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    try
    {
      return (theValue < other->getLongValue() ?
              -1 :
              (theValue == other->getLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;

  bool isNaN() const { return false; }
};


/*******************************************************************************
  class ByteItem
********************************************************************************/
class ByteItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_byte theValue;

protected:
  ByteItem(xs_byte aValue) : theValue(aValue) {}

  ByteItem() {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  xs_int getIntValue() const { return static_cast<xs_int>(theValue); }

  xs_short getShortValue() const { return static_cast<xs_short>(theValue); }

  xs_byte getByteValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_BYTE; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return theValue;
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getLongValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    try
    {
      return (theValue < other->getLongValue() ?
              -1 :
              (theValue == other->getLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  bool isNaN() const { return false; }

  zstring show() const;
};


/*******************************************************************************
  class UnsignedLongItem
********************************************************************************/
class UnsignedLongItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_ulong theValue;

protected:
  UnsignedLongItem(xs_ulong aValue) : theValue(aValue) {}

  UnsignedLongItem() {}

 public:
  bool isNaN() const { return false; }

  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_uinteger getUnsignedIntegerValue() const;

  xs_ulong getUnsignedLongValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_UNSIGNED_LONG; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getUnsignedLongValue();
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue() == other->getDecimalValue();
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    try
    {
      return (theValue < other->getUnsignedLongValue() ?
              -1 :
              (theValue == other->getUnsignedLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class UnsignedIntItem
********************************************************************************/
class UnsignedIntItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_uint theValue;

protected:
  UnsignedIntItem(xs_uint aValue) : theValue(aValue) {}

  UnsignedIntItem() {}

public:
  bool isNaN() const { return false; }

  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_uinteger getUnsignedIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  xs_ulong getUnsignedLongValue() const { return static_cast<xs_ulong>(theValue); }

  xs_uint getUnsignedIntValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_UNSIGNED_INT; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return theValue;
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0 ) const
  {
    try
    {
      return theValue == other->getUnsignedLongValue();
    }
    catch (error::ZorbaError&)
    {
      try
      {
        return getLongValue() == other->getLongValue();
      }
      catch (error::ZorbaError&)
      {
        return getDecimalValue() == other->getDecimalValue();
      }
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    try
    {
      return (theValue < other->getUnsignedLongValue() ?
              -1 :
              (theValue == other->getUnsignedLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class UnsignedShortItem
********************************************************************************/
class UnsignedShortItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_ushort theValue;

protected:
  UnsignedShortItem(xs_ushort aValue) : theValue(aValue) {}

  UnsignedShortItem() {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_uinteger getUnsignedIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  xs_ulong getUnsignedLongValue() const { return static_cast<xs_ulong>(theValue); }

  xs_uint getUnsignedIntValue() const { return static_cast<xs_uint>(theValue); }

  xs_ushort getUnsignedShortValue() const { return theValue; }

  bool isNaN() const { return false; }

  SchemaTypeCode getTypeCode() const { return XS_UNSIGNED_SHORT; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0 ) const
  {
    try
    {
      return theValue == other->getUnsignedLongValue();
    }
    catch (error::ZorbaError&)
    {
      try
      {
        return getLongValue() == other->getLongValue();
      }
      catch (error::ZorbaError&)
      {
        return getDecimalValue() == other->getDecimalValue();
      }
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* collation = 0) const
  {
    try
    {
      return (theValue < other->getUnsignedLongValue() ?
              -1 :
              (theValue == other->getUnsignedLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class UnsignedByteItem
********************************************************************************/
class UnsignedByteItem : public AtomicItem
{
  friend class BasicItemFactory;
  friend class AtomicItem;

protected:
  xs_ubyte theValue;

protected:
  UnsignedByteItem(xs_ubyte aValue) : theValue(aValue) {}

  UnsignedByteItem() : theValue(0) {}

public:
  xs_decimal getDecimalValue() const;

  xs_integer getIntegerValue() const;

  xs_uinteger getUnsignedIntegerValue() const;

  xs_long getLongValue() const { return static_cast<xs_long>(theValue); }

  xs_ulong getUnsignedLongValue() const { return static_cast<xs_ulong>(theValue); }

  xs_uint getUnsignedIntValue() const { return static_cast<xs_uint>(theValue); }

  xs_ushort getUnsignedShortValue() const { return static_cast<xs_ushort>(theValue); }

  xs_ubyte getUnsignedByteValue() const { return theValue; }

  bool isNaN() const { return false; }

  SchemaTypeCode getTypeCode() const { return XS_UNSIGNED_BYTE; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const
  {
    return static_cast<uint32_t>(theValue);
  }

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0 ) const
  {
    try
    {
      return theValue == other->getUnsignedLongValue();
    }
    catch (error::ZorbaError&)
    {
      try
      {
        return getLongValue() == other->getLongValue();
      }
      catch (error::ZorbaError&)
      {
        return getDecimalValue() == other->getDecimalValue();
      }
    }
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    try
    {
      return (theValue < other->getUnsignedLongValue() ?
              -1 :
              (theValue == other->getUnsignedLongValue() ? 0 : 1));
    }
    catch (error::ZorbaError&)
    {
      return getDecimalValue().compare(other->getDecimalValue());
    }
  }

  store::Item_t getEBV() const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class BooleanItem
********************************************************************************/
class BooleanItem : public AtomicItem
{
 friend class BasicItemFactory;

protected:
  xs_boolean theValue;

protected:
  BooleanItem(xs_boolean aValue) : theValue(aValue) {}

  BooleanItem() {}

public:
  xs_boolean getBooleanValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_BOOLEAN; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return other->getBooleanValue() == theValue;
  }

  long compare(
        const Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0) const
  {
    return (theValue == other->getBooleanValue() ?
            0 :
            (theValue == false ? -1 : 1));
  }

  store::Item_t getEBV( ) const;

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class Base64BinaryItem
********************************************************************************/
class Base64BinaryItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  xs_base64Binary theValue;

protected:
  Base64BinaryItem(xs_base64Binary aValue) : theValue(aValue) {}

  Base64BinaryItem() {}

public:
  xs_base64Binary getBase64BinaryValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_BASE64BINARY; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0 ) const
  {
    return theValue.equal(other->getBase64BinaryValue());
  }

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class HexBinaryItem
********************************************************************************/
class HexBinaryItem : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  xs_hexBinary theValue;

protected:
  HexBinaryItem(xs_hexBinary aValue) : theValue(aValue) {}

  HexBinaryItem() {}

public:
  xs_hexBinary getHexBinaryValue() const { return theValue; }

  SchemaTypeCode getTypeCode() const { return XS_HEXBINARY; }

  store::Item* getType() const;

  uint32_t hash(long timezone = 0, const XQPCollator* aCollation = 0) const;

  bool equals(
        const store::Item* other,
        long timezone = 0,
        const XQPCollator* aCollation = 0 ) const
  {
    return theValue.equal(other->getHexBinaryValue());
  }

  zstring getStringValue() const;

  void getStringValue2(zstring& val) const;

  void appendStringValue(zstring& buf) const;

  zstring show() const;
};


/*******************************************************************************
  class ErrorItem

  An ErrorItem obj stores a pointer to a ZorbaError obj. The ZorbaError obj is
  not created by the store, but once a pointer to it is set inside an ErrorItem
  obj, the ErrorItem obj assumes ownership of the ZorbaError. The getError()
  method returns the zorbaError pointer, but if the caller needs to access the
  ZobaError obj beyond the lifetime of the ErrorItem, they must make a deep
  copy of it.

  This design is due to the fact that ZorbaError class is visible to the store
  lib, but subclasses of ZorbaError (like ZorbaUserError) may not be visible,
  so ErrorItem cannot do much with an error obj other than storing a pointer
  to it (and calling the virtual free() method on that pointer when the
  ErrorItem gets destroyed).
********************************************************************************/
class ErrorItemNaive : public AtomicItem
{
  friend class BasicItemFactory;

protected:
  error::ZorbaError * theError;

protected:
  ErrorItemNaive(error::ZorbaError* error) : theError(error) {}

public:
  ~ErrorItemNaive();

  bool isError() const  { return true; }

  error::ZorbaError* getError() const { return theError; }

  SchemaTypeCode getTypeCode() const { return ZXSE_ERROR; }

  zstring show() const;

protected:
  // Disable copy
  ErrorItemNaive(const ErrorItemNaive& other);
  ErrorItemNaive& operator=(const ErrorItemNaive& other);
};

} // namespace simplestore
} // namespace zorba
#endif /* ZORBA_STORE_ATOMIC_ITEMS_H */

/*
 * Local variables:
 * mode: c++
 * End:
 */
 /* vim:set et sw=2 ts=2: */
