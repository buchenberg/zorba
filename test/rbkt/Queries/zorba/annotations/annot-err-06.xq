declare namespace ann = "http://www.zorba-xquery.com/annotations";

declare namespace random = "http://zorba.io/modules/random";

declare %ann:nondeterministic %ann:nondeterministic function random:random() as xs:integer external;


random:random()
