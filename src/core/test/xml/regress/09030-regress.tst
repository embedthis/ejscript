/*
    Was failing to parse the ".<"
 */

var xml: XML = <para><def>Circle</def> Geometric figure.</para>
assert(xml.def == "Circle")
