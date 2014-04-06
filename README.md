HexaNoise
=========

[![Build status](https://travis-ci.org/Nocte-/hexanoise.svg)](https://travis-ci.org/Nocte-/hexanoise) [![Coverage Status](https://coveralls.io/repos/Nocte-/hexanoise/badge.png)](https://coveralls.io/r/Nocte-/hexanoise)

This is a library for generating 2D coherent noise.  It compiles a noise
definition script to OpenCL, or it can run it through a simple interpreter in
case no OpenCL device is available.  A compiled script can then generate noise
for a given area and resolution.

Examples
--------

The Hexahedra Noise Definition Language is a functional language that
supports higher-order functions, lambdas, global variables, and named
functions.  The first example generates billow noise:

    scale(100):fractal(perlin:abs,2)

![scale(100):fractal(perlin:abs,2)](http://i1.minus.com/i8fsgPpfFIngP.png)

First, the noise is scaled so we can see some detail (it's drawn at a
resolution of 1 pixel per unit).  The base of the billow noise is created by
running the output of Perlin noise through the abs function.  It is then
combined into two octaves of fractal noise.  (To make ridge noise, use 
`perlin:abs:neg`.)

Another example, using a checkerboard pattern as the input to 'blend', to
alternate between concentric circles (the sine of the distance to origin) and
some classic Perlin noise:

    scale(250):checkerboard:blend
    (
        scale(10):distance:sin ,
        scale(100):fractal(perlin,7)
    )

![scale(250):checkerboard:blend(scale(10):distance:sin,scale(100):fractal(perlin,7))](http://i1.minus.com/iSeaHIsY9wI4G.png)


Documentation
-------------

A full description of HNDL is available as a [wiki page](https://github.com/Nocte-/hexahedra/wiki/Noise-definition-language).

License
-------

HexaNoise is free software: you can distribute it and/or modify it under the
terms of the [MIT license](http://opensource.org/licenses/MIT).

