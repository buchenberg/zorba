import module namespace xqddf-test = "http://www.zorba-xquery.com/modules/xqddf/test" at "xqddf_prolog.xqlib";
import module namespace ddl = "http://www.zorba-xquery.com/modules/store/static/collections/ddl";
import module namespace dml = "http://www.zorba-xquery.com/modules/store/static/collections/dml";
import module namespace ic_ddl = "http://www.zorba-xquery.com/modules/store/static/integrity_constraints/ddl";

ddl:create-collection($xqddf-test:white-collection);

ic_ddl:activate-integrity-constraint($xqddf-test:eic1);
ic_ddl:activate-integrity-constraint($xqddf-test:eic2);

{
<newline>
</newline>
},
{
for $i in fn:doc("auction.xml")//item
return fn:string(($i/description/parlist/listitem/text)[1])
},
{
<newline> a
</newline>
},
{
for $i in fn:doc("auction.xml")//item
return $i/name
},
{
<newline> a
</newline>
},
{
for $i in fn:doc("auction.xml")//item
return 
    {{$i/name},
    {dml:insert-nodes($xqddf-test:white-collection, (copy $copyi := $i modify () return $copyi));}}
},
{
<newline> a
</newline>
},
{
dml:collection($xqddf-test:white-collection)/description/parlist/listitem/text
}
