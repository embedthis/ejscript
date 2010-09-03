/*
  	Test namespaces qualification of types, functions and vars
 */
use strict

public namespace MySpace = "debug_space"

MySpace class Shape {
	MySpace static function size(): Number {
		return 77
	}
}

use namespace MySpace

assert(Shape.size() == 77)
