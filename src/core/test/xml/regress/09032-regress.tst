/*
    Test xml node replacement
 */

var x : XML = <parent>
    <one>1</one>
    <two>2</two>
</parent>

var three1 : XML = <three>3</three>
var three2 : XML = <three>33</three>

x['three'] = three1
x['three'] = three2

assert(x.toString() == '<parent><one>1</one><two>2</two><three>33</three></parent>')
