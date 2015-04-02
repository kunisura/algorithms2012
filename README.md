# algorithms2012

Source programs of the Numberlink solver and the Slitherlink solver/generator
introduced in:

> [Ryo Yoshinaka, Toshiki Saitoh, Jun Kawahara, Koji Tsuruma, Hiroaki Iwashita,
  and Shin-ichi Minato,
  Finding All Solutions and Instances of Numberlink and Slitherlink by ZDDs,
  Algorithms 5(2), 176-213 (2012).](http://www.mdpi.com/1999-4893/5/2/176)

You can also find an improved version of Numberlink solver (`ddnumlin`) in
[TdZdd](https://github.com/kunisura/TdZdd).
The Numberlink instance generator is available separately at
[Jun Kawahara's repository](https://github.com/junkawahara/numberlink_gen).

## Programs

* `znumlin`: Numberlink solver
* `zslilin`: Slitherlink solver
* `zsligen`: Slitherlink generator

## Requirements

The programs are tested on 64-bit Linux and should be compiled on Modern
Linux or Linux-like environment with GCC and GNU Make.
[CUDD: CU Decision Diagram Package](http://vlsi.colorado.edu/~fabio/CUDD/)
is required to compile `zsligen`.
