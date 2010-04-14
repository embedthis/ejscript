var p = new Path(".")
for each (f in p.find(".", "*", true)) {
    print(f)
}
