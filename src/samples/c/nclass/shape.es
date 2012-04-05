/*
    Shape.es - Sample module (nclass) with a Shape class. 
 */

module nclass {

	class Shape {

		native var x: num
		native var y: num
		native var height: num
		native var width: num

		public native function Shape(height: num, width: num)

		public native function area(): num 

		public function moveTo(x: num, y: num): void {
            this.x = x
            this.y = y
		}
	}
}
