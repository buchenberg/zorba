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

#ifndef ZORBA_FILEMODULE_FILEFUNCTION_H
#define ZORBA_FILEMODULE_FILEFUNCTION_H

#include <zorba/external_function.h>

namespace zorba { namespace filemodule {

  class FileModule;

  class FileFunction : public NonePureStatelessExternalFunction
  {
    private:
      static const std::string FILE_SCHEMA;

#ifdef WIN32
      static bool
      isValidDriveSegment(
          String& aString);
#endif

    protected:
      const FileModule* theModule;

      static void
      throwError(
          const std::string aErrorMessage,
          const XQUERY_ERROR& aErrorType);

      static String
      getOneStringArg(
          const StatelessExternalFunction::Arguments_t& aArgs,
          int aPos);

      static String
      getFilePathString(
          const StaticContext* aSctxCtx,
          const StatelessExternalFunction::Arguments_t& aArgs,
          int aPos);

    public:
      FileFunction(const FileModule* aModule);
      ~FileFunction();

      virtual String
      getURI() const;

  };

} /* namespace filemodule */ } /* namespace zorba */

#endif /* ZORBA_FILEMODULE_FILEFUNCTION_H */