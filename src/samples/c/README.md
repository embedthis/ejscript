Ejscript C samples
===

Samples:
---
    function   - Simple way to define a global funcion

    composite  - Composite native class. This is a native class where instance state is stored as a composite
                 C structure. Offers the most optimized memory storage for classes. Ideal for if you have very large
                 numbers of object instances or types with lots of properties. See also the native sample.

    evalFile   - Simple one line Ejscript embedding to evaluate a single script file.

    evalModule - Simple one line Ejscript embedding to evaluate a pre-compiled module file.

    evalScript - Simple one line Ejscript embedding to evaluate a script literal.

    naitve     - Native class. This is a native class that demonstrates binding C functions to JavaScript methods.
                 It creates a loadable module containing the native class. See also the composite sample.

## To Build:
    make

## To Run:
    See each sample
