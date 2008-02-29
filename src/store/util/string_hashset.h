#ifndef XQP_DEFAULT_STORE_STRING_HASHSET
#define XQP_DEFAULT_STORE_STRING_HASHSET

#include <zorba/common/common.h>
#include <zorbatypes/xqpstring.h>

#include "store/util/handle_hashset.h"


namespace xqp
{

class StringPool : public HandleSet<xqpStringStore, StringPool>
{
public:
  static const ulong DEFAULT_POOL_SIZE;
  static const float DEFAULT_LOAD_FACTOR;


  static bool equal(const xqpStringStore* s1, const xqpStringStore* s2)
  {
    return s1->byteEqual(*s2);
  }

  static uint32_t hash(const xqpStringStore* s)
  {
    return s->hash();
  }

public:
  StringPool(ulong size) : HandleSet<xqpStringStore, StringPool>(size) {};

  ~StringPool();

  bool insertc(const char* str, xqpStringStore_t& outStr);

protected:
  void garbageCollect();
};


}

#endif
