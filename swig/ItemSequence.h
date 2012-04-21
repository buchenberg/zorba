/*
 * Copyright 2006-2012 The FLWOR Foundation.
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
#ifndef API_ITEM_SEQUENCE_H
#define API_ITEM_SEQUENCE_H

class ItemSequence
{
  friend class Collection;
  friend class CollectionManager;
  friend class StaticCollectionManager;
private:
  zorba::ItemSequence_t theItemSequence;

public:
  ItemSequence(const ItemSequence& aItemSequence) : theItemSequence(aItemSequence.theItemSequence) {}
  ItemSequence(zorba::ItemSequence_t aItemSequence) : theItemSequence(aItemSequence) {}
  Iterator getIterator();

};

#endif