(:
 : Copyright 2006-2011 The FLWOR Foundation.
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

(:~
 : Zorba FOTS driver feedback
 : @author Sorin Nasoi
 :)

module namespace feedback =
  "http://www.zorba-xquery.com/fots-driver/feedback";

declare namespace ann =
  "http://www.zorba-xquery.com/annotations";

declare namespace fots =
  "http://www.w3.org/2010/09/qt-fots-catalog";

declare namespace err =
  "http://www.w3.org/2005/xqt-errors";

declare function feedback:check-pass(
  $result           as item()*,
  $testCaseName     as xs:string?,
  $testSetName      as xs:string?,
  $expectedFailure  as element(Test)?
) as xs:boolean
{
(: if the exact error code was not found, report the test as 'Pass' 
   with an attribute correctError=false :)
  let $resultTestRun as xs:boolean := 
    (empty($result) or
     (exists($result) and
      contains(string-join($result/fots:errors,''), "Expected error:") and
      contains(string-join($result/fots:errors,''), "Found error:")))

  let $expectedFailure as xs:boolean := 
    if (exists($expectedFailure))
    then fn:true()
    else fn:false()

(:if test PASSED and was expected to FAIL OR test FAILED and wasn't expected to
  FAIL => report as FAIL, a.k.a. regression;
  otherwise report as PASS:)
  return ($resultTestRun eq not($expectedFailure))
};


declare %ann:sequential function feedback:pass(
  $case             as element(fots:test-case),
  $result           as item()*,
  $zorbaQuery       as xs:string,
  $env              as element(fots:environment)?,
  $duration         as xs:dayTimeDuration,
  $verbose          as xs:boolean,
  $expectedFailure  as xs:boolean
) as element(fots:test-case)?
{
  if ($expectedFailure)
  then feedback:pass-expected-FOTS-failure( $case,
                                            $result,
                                            $zorbaQuery,
                                            $env,
                                            $verbose)
  else feedback:pass( $case,
                      $result,
                      $zorbaQuery,
                      $env,
                      $duration,
                      $verbose)
};


(:~
 : Feedback on a test case that failed but is marked as EXPECTED_FOTS_FAILURE.
 :
 : @param $case test case.
 : @return the test case after certain information was added.
 :)
declare  %private %ann:sequential function feedback:pass-expected-FOTS-failure(
  $case             as element(fots:test-case),
  $result           as item()*,
  $zorbaQuery       as xs:string,
  $env              as element(fots:environment)?,
  $verbose          as xs:boolean
) as element(fots:test-case)?
{
  variable $info := "Test case failed but it is marked with EXPECTED_FOTS_FAILURE in test/fots/CMakeLists.txt";

  if($verbose)
  then {
    let $tmp := $case
    return {
      insert node
      attribute result{'pass'}
      as last into $tmp;
      
      insert node
      attribute comment{$info}
      as last into $tmp;
      
      insert node
        <fots:info>
          {$env}
          <fots:query>{$zorbaQuery}</fots:query>
          {($result/fots:expected-result,
            $result/fots:result,
            $result/fots:errors)}
        </fots:info>
      as last into $tmp;
      
      delete node $tmp/fots:description;
      delete node $tmp/fots:created;
      delete node $tmp/fots:result;

      $tmp
    }
  }
  else <fots:test-case  name="{data($case/@name)}"
                        result="pass"
                        comment="{$info}" />
};


(:~
 : Gives feedback on a test case run with success.
 :
 : @param $case test case.
 : @return the test case after certain information was added.
 :)
declare %private %ann:sequential function feedback:pass(
  $case             as element(fots:test-case),
  $result           as item()*,
  $zorbaQuery       as xs:string,
  $env              as element(fots:environment)?,
  $duration         as xs:dayTimeDuration,
  $verbose          as xs:boolean
) as element(fots:test-case)?
{
  if($verbose)
  then {
    let $tmp := $case
    return {
      insert node
      attribute result{'pass'}
      as last into $tmp;
      
      if(exists($result/fots:errors)) then
        insert node
        attribute correctError{'false'}
        as last into $tmp;
      else ();

      if(exists($result/fots:errors)) then
        insert node
        attribute comment{$result/fots:errors}
        as last into $tmp;
      else ();

      insert node
      attribute executionTime{$duration}
      as last into $tmp;

      insert node
        <fots:info>
          {$env}
          <fots:query>{$zorbaQuery}</fots:query>
          {$result/descendant::*}
        </fots:info>
      as last into $tmp;

      delete node $tmp/description;
      delete node $tmp/created;
      delete node $tmp/fots:result;

      $tmp
    }
  }
  else if(empty($result/fots:errors))
  then <fots:test-case  name="{data($case/@name)}"
                        result="pass"
                        executionTime="{$duration}" />
  else <fots:test-case  name="{data($case/@name)}"
                        result="pass"
                        correctError="{empty($result/fots:errors)}"
                        executionTime="{$duration}" />
};


(:~
 : Gives feedback on a test case run without success.
 :
 : @param $case test case.
 : @return the test case after certain information was added.
 :)
declare %ann:sequential function feedback:fail(
  $case             as element(fots:test-case),
  $result           as item()*,
  $zorbaQuery       as xs:string,
  $testSetName      as xs:string?,
  $env              as element(fots:environment)?,
  $duration         as xs:dayTimeDuration,
  $verbose          as xs:boolean,
  $expectedFailure  as xs:boolean
) as element(fots:test-case)?
{
  trace($testSetName, "test set name");
  trace("above test case failed", "result");

  variable $info := "Test case passed but it is marked with EXPECTED_FOTS_FAILURE in test/fots/CMakeLists.txt";

  if ($verbose)
  then {
    let $tmp := $case
    return {
      insert node
      attribute result{'fail'}
      as last into $tmp;
      
      if ($expectedFailure)
      then
        insert node
        attribute comment{$info}
        as last into $tmp;
      else ();
      
      insert node
        <fots:info>
          {$env}
          <fots:query>{$zorbaQuery}</fots:query>
          {($result/fots:expected-result,
            $result/fots:result,
            $result/fots:errors)}
        </fots:info>
      as last into $tmp;
      
      delete node $tmp/fots:description;
      delete node $tmp/fots:created;
      delete node $tmp/fots:result;
      
      $tmp
    }
  }
  else if ($expectedFailure)
  then <fots:test-case name="{data($case/@name)}"
                       result="fail"
                       comment="{$info}"
                       executionTime="{$duration}"/>
  
  else <fots:test-case name="{data($case/@name)}"
                       result="fail"
                       executionTime="{$duration}"/>
};


(:~
 : Gives feedback on a test case that is not run (because its name is listed
 : in the $exceptedTestCases global var).
 :
 : @param $case test case.
 : @return the test case.
 :)
declare %ann:sequential function feedback:not-run(
  $case     as element(fots:test-case),
  $verbose  as xs:boolean
) as element(fots:test-case)?
{
  trace(data($case/@name), "processing test case :");
  trace("Above test case was not run.","");

  if ($verbose)
  then 
  {
    {
      (insert node attribute result {'notRun'} as last into $case,
       delete node $case/fots:description,
       delete node $case/fots:created);
      
      $case
    }
  }
  else
  {
    <fots:test-case name="{$case/@name}" result="notRun" />
  }
};


(:~
 : Gives feedback on a test case that is not run when dependencies are not met.
 :
 : @param $case test case.
 : @param $dependencyError test error returned by the dependency checking.
 : @return the test case.
 :)
declare %ann:sequential function feedback:not-applicable(
  $case             as element(fots:test-case),
  $env              as element(fots:environment)?,
  $dependencyError  as xs:string,
  $verbose          as xs:boolean
) as element(fots:test-case)? {
  trace(data($case/@name), "processing test case :");
  trace($dependencyError, "Dependency error :");

  if($verbose)
  then {
    let $tmp := $case
    return {
      insert node
      attribute result{'not applicable'}
      as last into $tmp;
      
      insert node
      attribute comment{$dependencyError}
      as last into $tmp;
      
      insert node
        <fots:info>{$env}</fots:info>
      as last into $tmp;
      
      delete node $tmp/fots:description;
      delete node $tmp/fots:created;
      
      $tmp
      }
  }
  else
    <fots:test-case name="{data($case/@name)}"
                    result="not applicable"
                    comment="{$dependencyError}" />
};