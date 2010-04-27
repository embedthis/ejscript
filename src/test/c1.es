/*
    Object.create
 */

var Shape = {
    getValue: function () {
        return this._value
    },
    setValue: function (_value) {
        this._value = _value
    }
}

var shape = Object.create(
    Shape, {
        _value: {
            value: "Hello ES5"
        }
    }
)
print(shape.getValue())
