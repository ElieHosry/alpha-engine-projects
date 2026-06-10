# Primitive Types

In Alpha Engine, some primitivate types have been renamed to contain more information about the amount of bits and bytes we want our variables to occupy.

```c
#include <cstdint>
typedef int8_t    s8;
typedef uint8_t   u8;
typedef int16_t   s16;
typedef uint16_t  u16;
typedef int32_t   s32;
typedef uint32_t  u32;
typedef int64_t   s64;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
```


`u8` stands for 'unsigned 8 bits'. `s8` stands for 'signed 8 bits'. 
`u16` then obviously stands for 'unsigned 16 bits', so on and so forth.
`f32` stands for '32-bit floating point' and `f64` stands for '64-bit floating point'. 

This technique is common in modern programming. 

You might ask, why not use the primitive types as is? Like `int` or `short`?

Doing it this way is not only clearer, but it also eliminates the need to change your primitive types based on different computer achitectures.

Consider that we declare a simple `int` like so:

```c
int i = 0; // How many bytes is i?
```

We might assume that an `int` is 4 bytes.
However, the reality is that this might not always be true.
Notice that in your programming class, quizzes always allow you to **assume** that `int` is 4 bytes!

The reality is that what `int` is depends on the compiler, which in turn depends on your computer architecture. This is stated in C++'s specifications:
> int - basic integer type. The keyword int may be omitted if any of the modifiers listed below are used. If no length modifiers are present, it's guaranteed to have a width of at least 16 bits. However, on 32/64 bit systems it is almost exclusively guaranteed to have width of at least 32 bits (see below). 
- [cppreference](https://en.cppreference.com/w/cpp/language/types)

Thankfully since C99, compilers need to implement the header file <cstdint> which provides typedefs that allows programmers to specify exactly how many bytes they want their variable to occupy (amongst other things).
Alpha Engine simply takes the typedefs it needs from that header file and shortened them.
