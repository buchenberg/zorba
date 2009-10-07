(:~
 : Module Name: eXcel-statistical
 : Module Version: 0.1
 : Date: August 1, 2009
 : Copyright:  2006-2008 The FLWOR Foundation. Licensed under the Apache License, Version 2.0.
 : Proprietary XQuery Extensions Used: None
 : XQuery Specification: January 2007
 : Module Overview: This is a library module offering the same set of functions
 : defined by Microsoft Excel into
 : @see http://office.microsoft.com/en-us/excel/CH062528311033.aspx
 : @author  Daniel Turcanu
 :)

module namespace  excel = "http://www.zorba-xquery.com/modules/excel/statistical" ;

import module namespace excel-math="http://www.zorba-xquery.com/modules/excel/math";
import module namespace excel-err="http://www.zorba-xquery.com/modules/excel/errors";

(:~
  Helper function for average function.
  It counts all non-empty arguments from the sequence.
  The string value of every argument is used for checking.
  
  @param $numbers sequence of values
  @return the count of non-empty string values
:)
declare function excel:count-non-empty( $numbers as xs:anyAtomicType* ) as xs:integer
{
  if(fn:empty($numbers)) then
    0
  else
    if(fn:string($numbers[1]) = "") then
      excel:count-non-empty(fn:subsequence($numbers, 2))
    else
      excel:count-non-empty(fn:subsequence($numbers, 2)) + 1
};

(:~
  Returns the average (arithmetic mean) of the arguments.
  Arguments can be empty values, otherwise must be castable to numeric.
  If sequence is empty then zero is returned.
  The sequence can be of any length.

  @see http://office.microsoft.com/en-us/excel/HP052089941033.aspx
  @param $numbers the sequence of numbers or empty values
  @return the sum of all numbers divided by the number of non-empty values
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:average ( $numbers as xs:anyAtomicType* ) as xs:anyAtomicType
{
  let $count := excel:count-non-empty($numbers) return
    if($count > 0) then
      excel-math:sum( (for $n in $numbers where fn:string($n) != "" return $n)) div $count
    else
      0 
};

(:~
  Counts the number of cells that contain numbers or values castable to numeric.
  
  @see http://office.microsoft.com/en-us/excel/HP052090261033.aspx
  @param $numbers the sequence of values, of any length
  @return the count of numbers
:)
declare function excel:count( $numbers as xs:anyAtomicType* )  as xs:integer
{
  if(fn:empty($numbers)) then
    0
  else
    if(excel-math:is-a-number($numbers[1])) then
      excel:count(fn:subsequence($numbers, 2)) + 1
    else
      excel:count(fn:subsequence($numbers, 2))
};

(:~
  Counts the empty values in a sequence.
  The empty values are the ones with string value "".
  The value 0 is not counted.
  
  @see http://office.microsoft.com/en-us/excel/HP052090281033.aspx
  @param $cells the sequence of values, of any length
  @return the count
:)
declare function excel:countblank( $cells as xs:anyAtomicType* ) as xs:integer
{
  if(fn:empty($cells)) then
    0
  else
    if(fn:string($cells[1]) = "") then
      excel:countblank(fn:subsequence($cells, 2)) + 1
    else
      excel:countblank(fn:subsequence($cells, 2))
};

(:~
  Returns the largest number in a sequence.
  
  @see http://office.microsoft.com/en-us/excel/HP052091701033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
            The sequence can be of any length.
  @return the max
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:max ( $numbers as xs:anyAtomicType* ) as xs:anyAtomicType
{
  fn:max( 
   (for $n_at in $numbers
    let $n := excel-math:cast-as-numeric($n_at)
    return $n
   )
   )
};

(:~
  Returns the smallest number in a sequence.
  
  @see http://office.microsoft.com/en-us/excel/HP052091761033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
            The sequence can be of any length.
  @return the min
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:min ( $numbers as xs:anyAtomicType* ) as xs:anyAtomicType
{
  fn:min( 
   (for $n_at in $numbers
    let $n := excel-math:cast-as-numeric($n_at)
    return $n
   )
   )
};

(:~
  Returns the median of the given numbers. 
  The median is the number in the middle of a set of numbers.
  Half the numbers have values that are greater than the median, 
  and half the numbers have values that are less than the median. 
 
  
  @see http://office.microsoft.com/en-us/excel/HP052091741033.aspx
  @param $numbers the sequence of numbers, of any length
  @return for odd count of numbers return the number in the middle of the sorted sequence.
        For even count of numbers return the average of the two numbers in the middle.
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:median( $numbers as xs:anyAtomicType* ) as xs:anyAtomicType
{
  let $number_count := excel:count( $numbers )
  let $sorted_numbers := excel-math:sort-numbers( $numbers ) return
  if($number_count mod 2 != 0) then
    $sorted_numbers[$number_count idiv 2 + 1]
  else
    if($number_count = 0) then
      0
    else
      ($sorted_numbers[$number_count idiv 2] + $sorted_numbers[$number_count idiv 2 + 1] ) div 2
};

(:~
  Returns the most frequently occurring, or repetitive, value in a sequence.
  Arguments must be castable to numeric.
  
  @see http://office.microsoft.com/en-us/excel/HP052091831033.aspx
  @param $numbers the sequence of numbers, of any length
  @return the most occuring number
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
  @error XQP0021(errNA) if there are no duplicate numbers
:)
declare function excel:mode( $numbers as xs:anyAtomicType* ) as xs:anyAtomicType
{
  if( fn:empty($numbers)) then
    fn:error($excel-err:errNA, "Mode function: empty sequence")
  else
  let $result := 
  ( for $n_at in fn:distinct-values($numbers) 
    let $n := excel-math:cast-as-numeric($n_at)
    let $count := fn:count( (for $d in $numbers where fn:string($d) = fn:string($n) return $d) )
    where $count > 1 
    order by $count descending
    return $n
  ) return 
  if(fn:empty($result)) then
    fn:error($excel-err:errNA, "Mode function: no duplicate elements")
  else
    $result[1]
};

(:~
  Returns the k-th percentile of values in a sequence.
  If k is not a multiple of 1/(n - 1), 
    PERCENTILE interpolates to determine the value at the k-th percentile.  
  The function is computed by (max-min)*k + min
  
  #see http://office.microsoft.com/en-us/excel/HP052092111033.aspx
  @param $numbers the sequence of numbers, of any length
  @param $k_at the percentile, with value between 0 .. 1 inclusive
  @return the computed percentile
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
  @error XQP0021(errNum) if percentile is not between 0 .. 1
:)
declare function excel:percentile( $numbers as xs:anyAtomicType*, $k_at as xs:anyAtomicType) as xs:anyAtomicType
{
  let $k := excel-math:cast-as-numeric($k_at) return
  if($k < 0 or $k > 1) then
    fn:error($excel-err:errNum, "Percentile function: k must be a value between 0 and 1 inclusive")
  else
    let $max := excel:max($numbers)
    let $min := excel:min($numbers) return
    ($max - $min) * $k + $min
};



(: priority 1 :)

(:~
  Helper function for AVEDEV.
  Computes formula sum(abs(x - average)) for every x in $numbers
  
  @param $numbers the sequence of numbers or values castable to numeric.
      Sequence can be of any length.
  @param $average average of all numbers, computed with function AVERAGE
  @return the result of formula
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:sum_deviations($numbers as xs:anyAtomicType*, $average as xs:anyAtomicType) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    0
  else
    fn:abs(excel-math:cast-as-numeric($numbers[1]) - $average) + excel:sum_deviations(fn:subsequence($numbers, 2),$average)
};

(:~
  Returns the average of the absolute deviations of data points from their mean.
  The formula is sum(abs(x - average_x))/n, where n is the count of x in the sequence.
  
  @see http://office.microsoft.com/en-us/excel/HP052089931033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
      Sequence can be of any length from 1 up.
  @return the formula result
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:avedev($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  let $average := excel:average($numbers) return
  excel:sum_deviations($numbers, $average) div excel:count($numbers)
};

(:~
  Helper function for AVERAGEA.
  This function adds all values that are castable to numeric.
  
  @param $numbers a sequence of any values, any length
  @return the sum of numbers
:)
declare function excel:add-all-cells($numbers as xs:anyAtomicType*) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    0
  else
  if(excel-math:is-a-number($numbers[1])) then
    excel-math:cast-as-numeric($numbers[1]) + excel:add-all-cells(fn:subsequence($numbers, 2))
  else
  (: if(fn:string($numbers[1]) = "") then :)
    excel:add-all-cells(fn:subsequence($numbers, 2))
  (: else
    fn:error($excel-err:errValue, "Provided value is not a number or empty cell ", $numbers[1]) :)
};

(:~
  Calculates the average (arithmetic mean) of the values in the sequence of arguments.
  Arguments can be of any type.
  The numbers are added, and the sum is divided by the size of entire sequence.
  
  @see http://office.microsoft.com/en-us/excel/HP052089951033.aspx
  @param $numbers the sequence of values of any type.
     The sequence can be of any length, from 1 up.
  @return the result
:)
declare function excel:averagea($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  excel:add-all-cells($numbers) div fn:count($numbers)
};

(:~
  Counts the number of values that are not empty.
  Empty values are the one with string value "".
  
  @see http://office.microsoft.com/en-us/excel/HP052090271033.aspx
  @param $numbers the sequence of values of any type, any length
  @return the count of non-empty values
:)
declare function excel:counta($numbers as xs:anyAtomicType*) as xs:integer
{
  if(fn:empty($numbers)) then
    0
  else
  if(fn:string($numbers[1]) != "") then
    1 + excel:counta(fn:subsequence($numbers, 2))
  else
    excel:counta(fn:subsequence($numbers, 2))
};

(:~
  Returns the k-th largest value in a data set. 
  If n is the number of data points in a range, 
    then LARGE(array,1) returns the largest value, 
    and LARGE(array,n) returns the smallest value.

  @see http://office.microsoft.com/en-us/excel/HP052091511033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
            The sequence can be of any length, from 1 up.
  @param $k the position of largest value, with value from 1 to count of values
  @return the k-th largest value as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
  @error XQP0021(errNum) if the sequence is empty
  @error XQP0021(errNum) if k is not a value between 1 and the sequence size
:)
declare function excel:large($numbers as xs:anyAtomicType*, $k as xs:integer) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    fn:error($excel-err:errNum, "Large function: value list must not be empty")  
  else if($k > fn:count($numbers) or $k le 0) then
    fn:error($excel-err:errNum, "Large function: k must be between 1 and the count of numbers ", $k)
  else
    let $ordered_numbers :=
      (for $n in $numbers 
       let $nn := excel-math:cast-as-numeric($n)
       order by $nn descending
       return $nn
      ) return
     $ordered_numbers[$k]
};

(:~
  Returns the largest value in a list of arguments.
  In this implementation there is no difference between MAX and MAXA.
  
  @see http://office.microsoft.com/en-us/excel/HP052091711033.aspx
  @param $numbers the sequence of numbers or values castable to numeric
            The sequence can be of any length.
  @return the max
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:maxa($numbers as xs:anyAtomicType*) as xs:anyAtomicType
{
  excel:max($numbers)
};

(:~
  Returns the smallest value in a list of arguments.
  In this implementation there is no difference between MAX and MAXA.
  
  @see http://office.microsoft.com/en-us/excel/HP052091771033.aspx
  @param $numbers the sequence of numbers or values castable to numeric
            The sequence can be of any length.
  @return the min
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:mina($numbers as xs:anyAtomicType*) as xs:anyAtomicType
{
  excel:min($numbers)
};

(:~
  Returns the rank of a number in a list of numbers. 
  The rank of a number is its size relative to other values in a list. 
  (If you were to sort the list, the rank of the number would be its position.)
  RANK gives duplicate numbers the same rank.

  @see http://office.microsoft.com/en-us/excel/HP052092311033.aspx
  @param $x is the number whose rank you want to find
  @param $numbers the sequence of numbers or values castable to numbers
    The sequence can be of any length
  @param $order_ascending is boolean and its meaning is:
     If false, then rank the number as if the sequence was sorted in descending order. 
     If true, then rank the number as if the sequence was sorted in ascending order. 
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type    
:)
declare function excel:rank($x as xs:anyAtomicType, 
                            $numbers as xs:anyAtomicType*, 
                            $order_ascending as xs:boolean) as xs:decimal
{
  let $ordered_numbers :=
    if($order_ascending) then
    (for $n in $numbers 
     let $nn := excel-math:cast-as-numeric($n)
     order by $nn ascending
     return $nn
    )
    else
    (for $n in $numbers 
     let $nn := excel-math:cast-as-numeric($n)
     order by $nn descending
     return $nn
    )
  let $xnum := excel-math:cast-as-numeric($x) 
  let $rank :=
  ( for $i at $pos in $ordered_numbers
    where $xnum = $i or $order_ascending and $xnum < $i
                     or fn:not($order_ascending) and $xnum > $i
    return 
    if($xnum = $i) then $pos
    else if($pos = 1) then 0
    else
      ($pos - 1) + ($xnum - $ordered_numbers[$pos - 1]) div ($ordered_numbers[$pos] - $ordered_numbers[$pos - 1])
   )
    return 
    if(fn:empty($rank)) then
     fn:count($numbers)
    else
     $rank[1]
};

(:~
  This RANK function is same as the above, only that $order_ascending is set by default to false.
:)
declare function excel:rank($x as xs:anyAtomicType, 
                            $numbers as xs:anyAtomicType*) as xs:decimal
{
  excel:rank($x, $numbers, fn:false())
};

(:~
  Returns the rank of a value in a data set as a percentage of the data set.
  If x does not match one of the values in array, 
    PERCENTRANK interpolates to return the correct percentage rank. 
  The formula is uses: (RANK - 1) / (size - 1)
  
  @see http://office.microsoft.com/en-us/excel/HP052092121033.aspx
  @param $numbers the sequence of numbers or values castable to numbers.
     The sequence can be of any length, from 1 up.
  @param $x is the value for which you want to know the rank
  @return the percentage of rank. 
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
  @error XQP0021(errNum) if the sequence is zero length
:)
declare function excel:percentrank($numbers as xs:anyAtomicType*, $x as xs:anyAtomicType) as xs:decimal
{
  if(fn:empty($numbers)) then
    fn:error($excel-err:errNum, "Percentrank function: value list must not be empty")
  else  
    let $rank := excel:rank($x, $numbers, fn:true()) return
    if($rank = 0) then
      0
    else
      ($rank - 1) div (fn:count($numbers) - 1)
};

(:~
  Returns the quartile of a data set. 
  
  @see http://office.microsoft.com/en-us/excel/HP052092261033.aspx
  @param $numbers sequence of numbers or values castable to numbers.
       The sequence can be of any length, from 1 up.
  @param $quart one of the values 0, 1, 2, 3, 4 with meaning:
      0 = compute minimum value
      1 = compute first quartile (25th percentile)
      2 = compute median value (50th percentile)
      3 = compute third quartile (75th percentile)
      4 = compute maximum value
   @return the computed quartile, as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
  @error XQP0021(errNum) if the sequence is zero length
  @error XQP0021(errNum) if $quart is not one of the values 0, 1, 2, 3, 4
:)
declare function excel:quartile($numbers as xs:anyAtomicType*, $quart as xs:integer) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    fn:error($excel-err:errNum, "Quartile function: value list must not be empty")
  else  
  if($quart = 0) then
    excel:min($numbers)
  else
  if($quart = 1) then
    let $r := (fn:count($numbers) + 3) div 4
    let $rint := xs:integer($r)
    let $rrem := $r - $rint 
    let $sorted_numbers := excel-math:sort-numbers( $numbers ) return
      ($numbers[$rint + 1] - $numbers[$rint]) * $rrem + $numbers[$rint] 
  else
  if($quart = 2) then
    excel:median($numbers)
  else
  if($quart = 3) then
    let $r := (3 * fn:count($numbers) + 1) div 4
    let $rint := xs:integer($r)
    let $rrem := $r - $rint 
    let $sorted_numbers := excel-math:sort-numbers( $numbers ) return
      ($numbers[$rint + 1] - $numbers[$rint]) * $rrem + $numbers[$rint] 
  else
  if($quart = 4) then
    excel:max($numbers)
  else
    fn:error($excel-err:errNum, "Quartile function: quart should be between 0 and 4 :", $quart)
};

(:~
  Returns the k-th smallest value in a data set. 
  Use this function to return values with a particular relative standing in a data set.
  If n is the number of data points in array, SMALL(array,1) equals the smallest value, 
    and SMALL(array,n) equals the largest value. 

  @see http://office.microsoft.com/en-us/excel/HP052092661033.aspx
  @param $numbers sequence of numbers or values castable to numeric.
     The sequence can be of any length, from 1 up.
  @param $k is the position (from the smallest) in the sequence of data to return.
     Must have value between 1 and size of sequence.
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
  @error XQP0021(errNum) if the sequence is zero length
  @error XQP0021(errNum) if $k is not a value between 1 and the size of sequence
:)
declare function excel:small($numbers as xs:anyAtomicType*, $k as xs:integer) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    fn:error($excel-err:errNum, "Small function: value list must not be empty")
  else  
  if($k > fn:count($numbers) or $k le 0) then
    fn:error($excel-err:errNum, "Small function: k must be between 1 and the count of numbers ", $k)
  else
    let $ordered_numbers :=
      (for $n in $numbers 
       let $nn := excel-math:cast-as-numeric($n)
       order by $nn ascending
       return $nn
      ) return
     $ordered_numbers[$k]
};

(:~
  Not implemented in this module.
  It is implemented in statistical-zorba module.
:)
declare function excel:stdev($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  fn:error($excel-err:errNA, "Stdev function: not implemented")
};

(:~
  Not implemented in this module.
  It is implemented in statistical-zorba module.
:)
declare function excel:stdevp($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  fn:error($excel-err:errNA, "Stdevp function: not implemented")
};

(:~
  Helper function for VAR, VARA, VARP, VARPA and SLOPE.
  It computes formula sum((x - average_x)^2) for all x in $numbers.
  
  @param $numbers the sequence of numbers or values castable to numbers.
      The sequence can be of any length.
  @param $average the precomputed average over the sequence
  @return the result as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type 
:)
declare function excel:sumsq_deviations($numbers as xs:anyAtomicType*, $average as xs:anyAtomicType) as xs:anyAtomicType
{
  if(fn:empty($numbers)) then
    0
  else
    let $val := excel-math:cast-as-numeric($numbers[1]) - $average return
    $val * $val + excel:sumsq_deviations(fn:subsequence($numbers, 2),$average)
};

(:~
  Estimates variance based on a sample.
  The formula is sum(x - average_x)^2 / (n - 1)
  average_x is computed with AVERAGE function.
  n is the count of numbers from the sequence, excluding empty values
  
  @see http://office.microsoft.com/en-us/excel/HP052093301033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
        The sequence can be of any length, from 1 up.
  @return the variance, as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:var($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  let $average := excel:average($numbers) return
  excel:sumsq_deviations($numbers, $average) div (excel:count($numbers) - 1)
};

(:~
  Estimates variance based on a sample.
  The formula is sum(x - average_x)^2 / (n - 1)
  average_x is computed with AVERAGE function.
  n is the size of sequence, including empty values
  
  @see http://office.microsoft.com/en-us/excel/HP052093311033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
        The sequence can be of any length, from 1 up.
  @return the variance, as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:vara($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  let $average := excel:average($numbers) return
  excel:sumsq_deviations($numbers, $average) div (fn:count($numbers) - 1)
};

(:~
  Calculates variance based on the entire population.
  The formula is sum(x - average_x)^2 / n
  average_x is computed with AVERAGE function.
  n is the count of numbers from the sequence, excluding empty values
  
  @see http://office.microsoft.com/en-us/excel/HP052093321033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
        The sequence can be of any length, from 1 up.
  @return the variance, as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:varp($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  let $average := excel:average($numbers) return
  excel:sumsq_deviations($numbers, $average) div excel:count($numbers)
};

(:~
  Calculates variance based on the entire population.
  The formula is sum(x - average_x)^2 / n
  average_x is computed with AVERAGE function.
  n is the size of sequence, including empty values
  
  @see http://office.microsoft.com/en-us/excel/HP052093321033.aspx
  @param $numbers the sequence of numbers or values castable to numeric.
        The sequence can be of any length, from 1 up.
  @return the variance, as numeric type
  @error XQP0021(errValue) if the parameters cannot be casted to numeric type
:)
declare function excel:varpa($numbers as xs:anyAtomicType+) as xs:anyAtomicType
{
  let $average := excel:average($numbers) return
  excel:sumsq_deviations($numbers, $average) div fn:count($numbers)
};

(:~ 
  Moved from math module.
  Returns a subtotal in a sequence of numbers.
  The function applied is given by $function_num.
  
  @see http://office.microsoft.com/en-us/excel/HP052092881033.aspx
  @param $function_num defines the function to be applied on sequence values.
        The possible values are:
        1 or 101 = AVERAGE
        2 or 102 = COUNT
        3 or 103 = COUNTA
        4 or 104 = MAX
        5 or 105 = MIN
        6 or 106 = PRODUCT
        7 or 107 = STDEV
        8 or 108 = STDEVP
        9 or 109 = SUM
        10 or 110 = VAR
        11 or 111 = VARP
        
        In this implementation there is no difference between x and 10x
   @param %numbers the sequence of numbers or values castable to numeric.
      The sequence can be of any length.
   @return the function result, as numeric type
   @error depends on the function called
   @error XQP0021(errNum) if $function_num is not a value between 1 .. 11 or 101 .. 111
:)
declare function excel:subtotal($function_num as xs:integer, $numbers as xs:anyAtomicType*) as xs:anyAtomicType
{
  if($function_num = 1 or $function_num = 101) then
    excel:average($numbers)
  else 
  if($function_num = 2 or $function_num = 102) then
    excel:count($numbers)
  else
  if($function_num = 3 or $function_num = 103) then
    excel:counta($numbers)
  else
  if($function_num = 4 or $function_num = 104) then
    excel:max($numbers)
  else
  if($function_num = 5 or $function_num = 105) then
    excel:min($numbers)
  else
  if($function_num = 6 or $function_num = 106) then
    excel-math:product($numbers)
  else
  if($function_num = 7 or $function_num = 107) then
   excel:stdev($numbers)
  else
  if($function_num = 8 or $function_num = 108) then
    excel:stdevp($numbers)
  else
  if($function_num = 9 or $function_num = 109) then
    excel-math:sum($numbers)
  else
  if($function_num = 10 or $function_num = 110) then
    excel:var($numbers)
  else
  if($function_num = 11 or $function_num = 111) then
    excel:varp($numbers)
  else
    fn:error($excel-err:errNum, "Subtotal function: function_num should be between 1 and 11 or 101 and 111")
};

(: priority 2 :)

(:~
  Helper function for PROB function.
  Computes the sum over a sequence of numbers.
  Checks if the values are between 0 and 1.
  
  @param $prob_range the sequence of probabilities
  @return the sum of probabilities. This should be 1.
  @error XQP0021(errNum) if any probability is not between 0 and 1
  @error XQP0021(errValue) if any parameter is not castable to numeric
:)
declare function excel:sum_prob($prob_range as xs:anyAtomicType*) as xs:anyAtomicType
{
  if(fn:empty($prob_range)) then
    0
  else
  let $prob_num := excel-math:cast-as-numeric($prob_range[1]) return
  if($prob_num < 0 or $prob_num > 1) then
    fn:error($excel-err:errNum, "Prob function: prob values should be between 0 and 1 ", $prob_num)
  else
    $prob_num + excel:sum_prob(fn:subsequence($prob_range, 2))
};

(:~
  Helper function for PROB function.
  Checks the prob range and x range if they have the same number of values.
  Adds all probabilities corresponding to values between range_lower_limit and upper_limit.
  
  @param $x_range the sequence of x values
  @param $prob_range the sequence of probabilies associated to x values
  @param $range_lower_limit the lower limit of the range to compute the probability
  @param $upper_limit the upper limit of the range to compute the probability
  @error XQP0021(errValue) if any parameter is not castable to numeric
  @error XQP0021(errNum) if x_range and prob_range do not have the same number of values
:)
declare function excel:sum_prob_x($x_range as xs:anyAtomicType*,
                                 $prob_range as xs:anyAtomicType*,
                                 $range_lower_limit as xs:anyAtomicType,
                                 $upper_limit as xs:anyAtomicType) as xs:anyAtomicType
{
  if(fn:empty($x_range) and fn:not(fn:empty($prob_range))) then
    fn:error($excel-err:errNum, "Prob function: x range and prob range should have the same number of elements")
  else
  if(fn:empty($prob_range) and fn:not(fn:empty($x_range))) then
    fn:error($excel-err:errNum, "Prob function: x range and prob range should have the same number of elements")
  else
  if(fn:empty($prob_range) and fn:empty($x_range)) then
    0
  else
  let $x := excel-math:cast-as-numeric($x_range[1])
  let $this_prob :=
    if($x ge $range_lower_limit and $x le $upper_limit) then
        excel-math:cast-as-numeric($prob_range[1])
    else
        0 
   return
  $this_prob + excel:sum_prob_x(fn:subsequence($x_range, 2),
                               fn:subsequence($prob_range, 2),
                               $range_lower_limit,
                               $upper_limit)
                                                              
};

(:~
  Returns the probability that values in a range are between two limits.
  
  @see http://office.microsoft.com/en-us/excel/HP052092221033.aspx
  @param $x_range is the range of numeric values of x with which there are associated probabilities.
        This does not need to be ordered.
  @param $prob_range is a set of probabilities associated with values in x_range.
  @param $range_lower_limit is the lower bound on the value for which you want a probability.
  @param $upper_limit  is the upper bound on the value for which you want a probability.
  @return the probability of the entire range
  @error XQP0021(errNum) if any probability is not between 0 and 1
  @error XQP0021(errNum) if the sum of probabilities is not equal to 1
  @error XQP0021(errValue) if any parameter is not castable to numeric
  @error XQP0021(errNum) if x_range and prob_range do not have the same number of values
:)
declare function excel:prob($x_range as xs:anyAtomicType+,
                            $prob_range as xs:anyAtomicType+,
                            $range_lower_limit as xs:anyAtomicType,
                            $upper_limit as xs:anyAtomicType) as xs:anyAtomicType
{
  let $prob_sum := excel:sum_prob($prob_range) return
  if($prob_sum != 1) then
    fn:error($excel-err:errNum, "Prob function: prob sum should equal 1")
  else
    excel:sum_prob_x($x_range, $prob_range, 
                    excel-math:cast-as-numeric($range_lower_limit), 
                    excel-math:cast-as-numeric($upper_limit))
};

(:~
  This is the same as above, only that upper_limit is not specified.
  The probability is computed only for range_lower_limit.
:)
declare function excel:prob($x_range as xs:anyAtomicType+,
                            $prob_range as xs:anyAtomicType+,
                            $range_lower_limit as xs:anyAtomicType) as xs:anyAtomicType
{
  excel:prob($x_range, $prob_range, $range_lower_limit, $range_lower_limit)
};

(:~
  Helper function for SLOPE function.
  It computes the formula:
  sum((x - average_x)(y - average_y))
  where average_x and average_y are computed with AVERAGE function.
  
  @param $x_numbers the sequence of x numbers
  @param $x_average the precomputed AVERAGE over the x_numbers
  @param $y_numbers the sequence of y numbers
  @param $y_average the precomputed AVERAGE over the y_numbers
  @return the formula result, as numeric type
  @error XQP0021(errValues) if any parameter cannot be casted to numeric
  @error XQP0021(errNA) if there are different numbers of x's and y's
:)
declare function excel:sum_x_y_deviations($x_numbers as xs:anyAtomicType*, 
                                          $x_average as xs:anyAtomicType,
                                          $y_numbers as xs:anyAtomicType*, 
                                          $y_average as xs:anyAtomicType) as xs:anyAtomicType
{
  if(fn:empty($x_numbers) and fn:not(fn:empty($y_numbers))) then
    fn:error($excel-err:errNA, "Slope function: different number of x's and y's")
  else
  if(fn:empty($y_numbers) and fn:not(fn:empty($x_numbers))) then
    fn:error($excel-err:errNA, "Slope function: different number of x's and y's")
  else
  if(fn:empty($x_numbers) and fn:empty($y_numbers)) then
    0
  else
    (excel-math:cast-as-numeric($x_numbers[1]) - $x_average) * 
    (excel-math:cast-as-numeric($y_numbers[1]) - $y_average) + 
    excel:sum_x_y_deviations(fn:subsequence($x_numbers, 2),$x_average,
                         fn:subsequence($y_numbers, 2),$y_average)
};

(:~
  Returns the slope of the linear regression line through data points in known_y's and known_x's.
  The slope is the vertical distance divided by the horizontal distance between 
    any two points on the line, which is the rate of change along the regression line.
  It computes the formula:
  sum((x - average_x)(y - average_y)) / sum((x - average_x)^2)
  where average_x and average_y are computed with AVERAGE function.
  
  @see http://office.microsoft.com/en-us/excel/HP052092641033.aspx
  @param $known_y the sequence of y numbers.
     The sequence can be of any length, from 1 up.  
  @param $known_x the sequence of x numbers.
     The sequence can be of any length, from 1 up.  
  @return the slope value, as numeric type
  @error XQP0021(errValues) if any parameter cannot be casted to numeric
  @error XQP0021(errNA) if there are different numbers of x's and y's
  @error XQP0021(errNA) if any sequence is empty
  @error XQP0021(errDiv0) if all x's are equal
:)
declare function excel:slope($known_y as xs:anyAtomicType+,
                       $known_x as xs:anyAtomicType+)
{
  if(fn:empty($known_y) or fn:empty($known_x)) then
    fn:error($excel-err:errNA, "Slope function: known_x and known_y cannot be empty sequences")
  else
  let $x_average := excel:average($known_x) 
  let $y_average := excel:average($known_y) 
  let $xsq_dev := excel:sumsq_deviations($known_x, $x_average) return
  if($xsq_dev = 0) then
    fn:error($excel-err:errDiv0, "Slope function: all x's are equal")
  else
  let $x_y_dev := excel:sum_x_y_deviations($known_x, $x_average, $known_y, $y_average) return
  $x_y_dev div $xsq_dev
};

(:~
  Returns a normalized value from a distribution characterized by mean and standard_dev.
  The formula is (x - mean) / standard_dev

  @see http://office.microsoft.com/en-us/excel/HP052092731033.aspx
  @param $x is the value you want to normalize
  @param $mean  is the arithmetic mean of the distribution.
  @param $standard_dev is the standard deviation of the distribution.
  @return the normalized x, as numeric type
  @error XQP0021(errValues) if any parameter cannot be casted to numeric
  @error XQP0021(errNum) if standard_dev is a value smaller than zero or equal
:)
declare function excel:standardize($x as xs:anyAtomicType,
                                   $mean as xs:anyAtomicType,
                                   $standard_dev as xs:anyAtomicType) as xs:double
{
  if($standard_dev le 0) then
    fn:error($excel-err:errNum, "Standardize function: standard_dev must be positive ", $standard_dev)
  else
    (excel-math:cast-as-numeric($x) - excel-math:cast-as-numeric($mean)) div excel-math:cast-as-numeric($standard_dev)
};
