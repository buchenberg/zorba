import module namespace ddl = "http://www.zorba-xquery.com/modules/store/static/collections/ddl";
import module namespace dml = "http://www.zorba-xquery.com/modules/store/static/collections/dml";
import module namespace ns = "http://example.org/datamodule/" at "collections.xqdata";

declare %sequential function local:test()
{
  ddl:create-collection(xs:QName("ns:test1"));
  dml:insert-nodes-last(xs:QName("ns:test1"), <a/>);
  dml:insert-nodes-last(xs:QName("ns:test1"), <b/>);
  dml:insert-nodes-last(xs:QName("ns:test1"), <c/>);
  exit returning dml:collection(xs:QName("ns:test1"));
};

local:test()
