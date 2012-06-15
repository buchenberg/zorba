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

#include "stdafx.h"
#include <iostream>
#include <string>

#include "util/base64_util.h"

using namespace std;
using namespace zorba;

struct test {
  char const *input;
  char const *expected;
};

///////////////////////////////////////////////////////////////////////////////

static int failures;

static bool assert_true( int no, char const *expr, int line, bool result ) {
  if ( !result ) {
    cout << '#' << no << " FAILED, line " << line << ": " << expr << endl;
    ++failures;
  }
  return result;
}

static void print_exception( int no, char const *expr, int line,
                             std::exception const &e ) {
  assert_true( no, expr, line, false );
  cout << "+ exception: " << e.what() << endl;
}

#define ASSERT_TRUE( NO, EXPR ) assert_true( NO, #EXPR, __LINE__, !!(EXPR) )

#define ASSERT_NO_EXCEPTION( NO, EXPR ) \
  try { EXPR; } \
  catch ( std::exception const &e ) { print_exception( NO, #EXPR, __LINE__, e ); } \
  catch ( ... ) { assert_true( NO, #EXPR, __LINE__, false ); }

#define ASSERT_EXCEPTION( NO, EXPR, EXCEPTION ) \
  try { EXPR; assert_true( NO, #EXPR, __LINE__, false ); } \
  catch ( EXCEPTION const& ) { }

///////////////////////////////////////////////////////////////////////////////}

static void test_decode( int no, string const &in, string const &expected ) {
  char out[ 1024 ];
  base64::size_type n;
  ASSERT_NO_EXCEPTION( no, n = base64::decode( in.data(), in.size(), out ) );
  ASSERT_TRUE( no, n == expected.size() );
  out[ n ] = '\0';
  ASSERT_TRUE( no, out == expected );
}

static void test_encode( int no, string const &in, string const &expected ) {
  char out[ 1024 ];
  base64::size_type const n = base64::encode( in.data(), in.size(), out );
  ASSERT_TRUE( no, n == expected.size() );
  out[ n ] = '\0';
  ASSERT_TRUE( no, out == expected );
}

///////////////////////////////////////////////////////////////////////////////

static test const encode_tests[] = {
  /* 0 */ { "Now is the time", "Tm93IGlzIHRoZSB0aW1l" },
  /* 1 */ { "Now is the time.", "Tm93IGlzIHRoZSB0aW1lLg==" },
  /* 2 */ { "Now is the time..", "Tm93IGlzIHRoZSB0aW1lLi4=" },

  { 0, 0 }
};

static test const decode_tests[] = {
  /* 3 */ { "Tm93IGlzIHRoZSB0aW1l", "Now is the time" },
  /* 4 */ { "Tm93IGlzIHRoZSB0aW1lLg==", "Now is the time." },
  /* 5 */ { "Tm93IGlzIHRoZSB0aW1lLi4=", "Now is the time.." },

  // incomplete Base64 encodings
  /* 6 */ { "Tm93IGlzIHRoZSB0aW1", "Now is the tim" },
  /* 7 */ { "Tm93IGlzIHRoZSB0aW", "Now is the ti" },
  /* 8 */ { "Tm93IGlzIHRoZSB0a", "Now is the t" },

  { 0, 0 }
};

namespace zorba {
namespace UnitTests {

int test_base64( int, char*[] ) {
  int test_no = 0;

  for ( test const *t = encode_tests; t->input; ++t, ++test_no )
    test_encode( test_no, t->input, t->expected );

  for ( test const *t = decode_tests; t->input; ++t, ++test_no )
    test_decode( test_no, t->input, t->expected );

  cout << failures << " test(s) failed\n";
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba

/* vim:set et sw=2 ts=2: */
