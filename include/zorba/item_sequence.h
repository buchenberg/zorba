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
#ifndef ZORBA_ITEM_SEQUENCE_API_H
#define ZORBA_ITEM_SEQUENCE_API_H

#include <zorba/config.h>
#include <zorba/api_shared_types.h>
#include <zorba/serializable.h>

namespace zorba { 

  /** \brief This interface represents an instance of the XQuery 1.0 and XPath 2.0 Data Model (XDM).
   *
   * See http://www.w3.org/TR/xpath-datamodel/.
   */
  class ZORBA_DLL_PUBLIC ItemSequence : public Serializable
  {
    public:
      /** \brief Destructor
       */
      virtual ~ItemSequence() { }

      /** \brief Get the next Item of the sequence.
       *
       * @param aItem the next Item of the sequence if true is returned by the function.
       * @return true if the sequence is not exhausted, false otherwise.
       * @throw ZorbaException if an error occured.
       */
      virtual bool next(Item& aItem) = 0;

      /** \brief The Serializable interface implementation.
       *
       * @param item The item returned if this function returns true.
       * @return true if the sequence is not exhausted, false otherwise.
       * @throw ZorbaException if an error occured.
       */
      virtual bool
      nextSerializableItem(Item& item);

  }; /* class ItemSequence */

} // namespace zorba
#endif
