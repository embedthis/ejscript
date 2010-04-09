
o = {}
o.step = "abc\ndef"
print(o.step)

print("o.step " + o.step);

str = serialize(o)
print("STR " + str)

o2 = deserialize(str)
print('o2.step')
dump(o2)


/*
{
    use default namespace intrinsic

    var o = 1
}


o = 2

print(o)
print(intrinsic::o)
*/
