// use strict;

class Foo
{
    public function hello() {
        print "Hello";
    }

    function run(func: Function) {
        func.call(this, []);
    }
}

var foo: Foo = new Foo

foo.run(function (): void { this.hello(); });
