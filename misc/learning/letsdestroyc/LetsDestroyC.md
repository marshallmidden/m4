# Let's Destroy C

I have a pet project I work on, every now and then. [CNoEvil](https://git.sr.ht/~shakna/cnoevil3/).

The concept is simple enough.

What if, for a moment, we forgot all the rules we know. That we ignore every good idea, and accept all the terrible ones. That nothing is off limits. Can we turn C into a new language? Can we do what Lisp and Forth let the over-eager programmer do, but in C?

---

## Some concepts

We're going to point out some definitions in other files - they're too big to inline into a blog post.

You can assume that all of these header definitions get collapsed into a single file, called `evil.h`.

We won't dwell on many C features. If they're not obvious to you, there's a lot of information at your fingertips to explain them. The idea here isn't to explain how C has moved on. It's to abuse it.

---

First of all, let's fix up a simple program:


```C

#include <stdio.h>

int main(int argc, char* argv[]) {

  printf("%s\n", "Hello, World!");

}


```

That's an awful lot of symbolic syntax.

Let's try and get rid of a little of that.

## Format

Format specifiers are incredibly useful in C. Allowing you to specify how many decimal places to put after a float, where to use commas when outputting numbers. Whether to use the locale specifier to get the right ```,``` or ```.``` syntax, etc.

But, for the general case, we don't need it. So we can make it disappear.

We can do this, thanks to a C11 feature, called `_Generic`, which is sort of like a type-based switch. It'll match against the first compatible type.

If we define `display_format` as a `_Generic` switch, like you can see in `evil_io.h`, then we can replace our printf with a very simple set of defines:

    #define display(x) printf(display_format(x), x)
    #define displayln(x) printf(display_format(x), x); printf("%s", "\r\n")

Now we can rewrite our program like this:

```C
#include "evil.h"

int main(int argc, char* argv[]) {
	displayln("Hello, World!");
}
```

There. That's a lot more high level. And it works correctly for a whole bunch of things other than strings, too.

## Main

We've got a fairly typical `main` definition here. But we can do better. We can hide `argc` and `argv`, and just assume the programmer knows they're implicitly available. Because there is nothing worse than implicit values.

In fact, we'll also silence the compiler that might complain if we don't end up using them to inspect commandline flags.

    #define Main int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv

Unfortunately, just defining our `Main` isn't enough. We need a couple more defines, which will come in extremely handy in the future. Just a couple symbol replacements.

    #define then ){
    #define end }

Now. That's better. We can now rewrite our program:

```C
#include "evil.h"

Main then
  displayln("Hello, World!");
end
```

Brilliant. Now it doesn't look like C. It still compiles like C. In fact, it should compile without warnings.

(Have a glance at `evil_flow.h` for a few more useful defines that mean we can escape the brace syntax and pretend that C works like Lua's syntax.)

## High Level Constructs

We've got a `Hello, World` that looks simple. It wasn't a hard path to get here. But we can do even better than that.

We can add in things people don't expect to exist in C at all.

Then we can start pretending our poor, abused little program is actually a higher level language than it is. And we haven't even broken any C syntax, which means we can safely and easily link against any other C library, even if it is a header-only library.

### Lambda

With a GNU-extension (it may or may not work under other compilers), we can easily write a `lambda`, and give C the ability to have anonymous functions. We still need to use C's function-pointer syntax, but that doesn't turn out too bad in practice.

    #define lambda(ret_type, _body) ({ ret_type _ _body _; })

There! Simple, isn't it? Well, maybe not entirely obvious how it works. (See `evil_lambda.h` for our full implementation.)

```C
#define EVIL_LAMBDA
#include "evil.h"

Main then
  int (*max)(int, int) = lambda(int,
  	                         (int x, int y) {
  	                         	return x > y ? x : y;
  	                         });

  displayln(max(1, 2));
end
```

We create a function pointer called max, which returns an int, and takes two int arguments. The lambda assigned to it matches. It returns the bigger of the two values with a simple one-liner.

You use it like you might expect, but `max` only exists inside main, and is ready to be passed to another function so you can start building up your functional tools.

### Coroutines

You can write proper coroutine systems for C. They tend to be big, and complicated and extremely helpful.

But we're doing the wrong thing.

So, apart from emitting some compile-time warnings, the crux of `evil_coroutine.h` is this magnificent madness:

```C
// Original macro hack by Robert Elder (c) 2016. Used against their advice, but with their permission.
#define coroutine() static int state=0; switch(state) { case 0:
#define co_return(x) { state=__LINE__; return x; case __LINE__:; }
#define co_end() }
```

By storing state and using a switch as a computer `GOTO`, you can now write functions that appear to be resumeable.

Like so:

```C
#define EVIL_COROUTINE
#include "evil.h"

int example() {
  static int i = 0;
  coroutine();
  While true then
    co_return(++i);
  end
  co_end();
  return i;
}

Main then
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
end
```

Despite being dangerous, and poorly thought through if you're insane enough to put this anywhere near production code, we are looking like we have coroutines.

Unfortunately, those damn braces are back again.

## Procs

Technically speaking, C doesn't have functions. Because functions are pure and have no side-effects, and C is one giant stinking pile of a side-effect.

What C has, is properly known as `procedures`. So let's reflect that when we redefine how we make them, to get ride of the braces:

```C
#define declare(_name, _ret, ...) _ret _name(__VA_ARGS__)
#define proc(_name, _ret, ...) _ret _name(__VA_ARGS__){
```

This fits in nicely with our existing `then` and `end` macros.

We put the return type right before any listing of arguments, and after the name, which can make it easier when reading over the definition or decleration.

It let's us change the above example into this marvelous little beauty:

```C
#define EVIL_COROUTINE
#include "evil.h"

declare(example, int);

proc(example, int)
  static int i = 0;
  coroutine();
  While true then
    co_return(++i);
  end
  co_end();
  return i;
end

Main then
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
  displayln(example());
end

```

That's better. It looks more consistent with the rest of our syntax, whilst still not breaking how C works at all.

We've practically abolished symbols in the final syntax. They're still there, but minimal. We haven't introduced any whitespace sensitivity, but we have simplified how it looks. Made it feel like a scripting language.

---

[CNoEvil](https://git.sr.ht/~shakna/cnoevil3/) goes a lot further than this. It adds introspection, a new assert library with it's own stacktrace format, hash routines and so on.

But this is a taste of how well you can screw up the C language with just a handful of overpowered macros.