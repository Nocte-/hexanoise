HexaNoise
=========

This is a library for generating 2D coherent noise.  It compiles a noise
definition script to either OpenCL or a simple interpreter.  A compiled
script can then generate noise for a given area and resolution.

The Hexahedra Noise Definition Language is a functional language that
supports higher-order functions, lambdas, global variables, and named
functions.  Here are a some examples:

    scale(100):fractal(perlin:abs,2)

![scale(100):fractal(perlin:abs,2)](http://i1.minus.com/i8fsgPpfFIngP.png)

    scale(250):checkerboard:blend(scale(10):distance:sin,scale(100):shift(1000,1000):fractal(perlin,7))

![scale(250):checkerboard:blend(scale(10):distance:sin,scale(100):shift(1000,1000):fractal(perlin,7))](http://i1.minus.com/iSeaHIsY9wI4G.png)

A description of HNDL is available as a [wiki page](https://github.com/Nocte-/hexahedra/wiki/Noise-definition-language).

