# Berns changelog

## 4.3.0

Version 4.3.0 contains a major refactor of string allocation functions,
introducing a series of functions for handling strings in Berns and pushing
memory allocation to Ruby by using Ruby's ruby_xmalloc, ruby_xfree, and
ruby_xrealloc.

## 4.2.0

Allow inlining `empty_value_to_attribute`.

## 4.1.2

Fix a memory leak in `void_element`.

## 4.1.1

Rightsize the allocated memory from `hash_value_to_attribute` before returning.

## 4.1.0

Instances of the `Berns::Builder` class can now be `#call`'ed multiple times,
letting instances of `Berns::Builder` act like template objects.

## 4.0.0

No code changes were made, but the required Ruby version has been increased to
>= 2.7.0 in keeping with [2.6's recent
end-of-life](https://www.ruby-lang.org/en/downloads/branches/).

## 3.5.0

Add the `#element` and `#void` methods to blocks evaluated by `Berns::Builder`.

## 3.4.0

Add the `Berns::Builder` class, enabling simpler template composition.

This class effectively works like a DSL for HTML template composition. For
example, to construct an `article` with a heading and paragraph in earlier
versions you might do something like this:

``` ruby
markup = Berns.article do
  content = Berns.h1 { 'Heading' }
  content + Berns.p { 'A paragraph.' }
end
```

And the same thing using the new `Berns::Builder` class:

``` ruby
template = Berns::Builder.new do
  article {
    h1 { 'Heading' }
    p { 'A paragraph.' }
  }
end

markup = template.call # => "<article><h1>Heading</h1><p>A paragraph.</p></article>"
```

This has two advantages over the previous approach:

1. There's no need to build the string as you go. Any `Berns` standard or void
   element method will automatically append to an internal buffer.
2. There's less repetition since you don't need to add `Berns` in front of each
   element method call.

This new class is heavily inspired by the likes of
[Papercraft](https://github.com/digital-fabric/papercraft),
[Markaby](https://github.com/markaby/markaby), and
[Arbre](https://github.com/activeadmin/arbre).

## 3.3.1

Remove the `std=c99` flag.

## 3.3.0

`Berns.sanitize` now removes HTML entities like `&lt;` as well as their
unescaped equivalents.

## 3.2.1

Tweak `Berns.sanitize` to return the original string if no modifications were
made.

Before:

``` example
clean      8.452M (± 1.8%) i/s -     42.530M in   5.033708s
dirty      7.411M (± 1.1%) i/s -     37.561M in   5.069121s
```

After:

``` example
clean     15.509M (± 1.4%) i/s -     78.419M in   5.057355s
dirty      7.515M (± 1.1%) i/s -     37.948M in   5.050435s
```

## 3.2.0

Rewrite `Berns.sanitize` in C. It's still incredibly naive and, as noted in the
README, should probably only be used with trusted strings.

Before:

``` example
clean      3.404M (± 0.8%) i/s -     17.251M in   5.069001s
dirty    690.392k (± 1.7%) i/s -      3.506M in   5.079129s
```

After:

``` example
clean      8.744M (± 1.1%) i/s -     43.800M in   5.009626s
dirty      7.473M (± 0.7%) i/s -     37.734M in   5.049443s
```

## 3.1.6

- Refactor the way `$CFLAGS` are added to use the
  `append_cflags` method provided by `mkmf`.
- Add a few more flags including `-flto`.
- Remove the unneeded `create_header`.

## 3.1.5

Several minor fixes including:

- Append to `$CFLAGS` instead of overwriting it.
- Set `-std=c99`
- Change optimization level to 2 from 3.
- Fix a case of variable shadowing.

## 3.1.4

Stop passing pointers to Ruby's Hash values. I'm not sure that this was ever a
good idea.

## 3.1.3

Replace calls to `calloc` with `strdup("")`, which seems more correct.

## 3.1.2

Fix two potential off-by-one errors when calculating string length.

## 3.1.1

Fix an error when hash arguments to `to_attribute` have a `false` value. Prior
to this release the value would be converted to a string but with this release
the attribute is dropped altogether, which was the behavior before v3.0.

## 3.1.0

Version 3.1.0 is a major refactor of the underlying C code introduced with
version 3.0.0 that benefits from everything learned in the initial
implementation.

The API has been split between internal and external functions. Internal
functions accept a mix of C types and Ruby object types but tend to return C
types. These are composed together into external functions that accept Ruby
objects as arguments and return Ruby strings.

In addition, HTML escaping is now powered by the excellent
[k0kubun/hescape](https://github.com/k0kubun/hescape) library written by Takashi
Kokubun.

## 3.0.6

Convert non-string blocks of content into strings. This allows, for example,
passing numeric objects as blocks of content to standard element methods without
first having to convert them to strings.

## 3.0.5

Fix a regression when content blocks are false. They should be treated the same
as if they are not there instead of throwing an error.

This allows the use of conditions in combination with content blocks e.g.

``` ruby
Berns.div { user_can_view? && "You can see this sometimes" }
```

## 3.0.4

Fix an `ArgumentError` when passing a nested empty hash to `to_attribute`.

## 3.0.3

Fix a buffer overflow error.

## 3.0.2

Ensure all returned strings are UTF-8 encoded.

## 3.0.1

Fix a regression when content blocks are nil. They should be treated the same as
if they are not there instead of throwing an error.

## 3.0.0

Version 3.0 is another mostly API-compatible refactor of Berns, this time in
blazing fast C! I debated simply calling this version 2.1.0 but because it's a
complete rewrite it didn't seem right to do a simple point release and there may
be corner cases that I've not accounted for in this new C-backed version.

Running the same benchmarks as from 2.0 but pitting 2.0 against 3.0 yields some
great speed improvements, particularly for the `empty` and `simple` cases.

*These benchmarks were performed on a desktop with a AMD Ryzen 5 3600X 6-Core
Processor running Linux Mint 20.1 and kernel 5.4.*

Before:

``` example
 empty      1.668M (± 0.6%) i/s -      8.356M in   5.011099s
simple    442.102k (± 1.3%) i/s -      2.214M in   5.008068s
nested    267.716k (± 0.4%) i/s -      1.357M in   5.068747s
```

After:

``` example
 empty      3.573M (± 1.2%) i/s -     17.881M in   5.005001s
simple    840.631k (± 0.6%) i/s -      4.253M in   5.059771s
nested    267.281k (± 0.5%) i/s -      1.347M in   5.037887s
```

With both empty and simple attributes we see performance effectively double, and
with nested attributes performance remains more or less the same.

This is another set of fairly contrived benchmarks, testing a singleton method,
`void` call, and `element` call against each other.

Before:

``` example
            br      3.061M (± 0.8%) i/s -     15.613M in   5.100154s
    void("br")      6.141M (± 1.4%) i/s -     30.990M in   5.047338s
element("div")      2.789M (± 0.6%) i/s -     14.171M in   5.080626s
```

After:

``` example
            br      8.155M (± 1.0%) i/s -     41.339M in   5.069681s
    void("br")      9.782M (± 1.5%) i/s -     49.096M in   5.020114s
element("div")      6.769M (± 1.1%) i/s -     33.983M in   5.021362s
```

Lastly, benchmarking `to_attributes` with the following hash as the only
argument shows about double the performance with 3.0.

``` ruby
ATTRS = { this: 'tag', should: 'work', data: { foo: 'bar', bar: { baz: 'foo' } } }.freeze
```

Before:

``` example
to_attributes    228.829k (± 1.3%) i/s -      1.159M in   5.065714s
```

After:

``` example
to_attributes    457.387k (± 1.2%) i/s -      2.305M in   5.041036s
```

## 2.0.0

Version 2.0 is a mostly API-compatible refactor of all of the core methods that
make up Berns. The goal is to improve performance, mostly using mutable strings
and inlining variables that were otherwise short lived.

In addition, the target Ruby version has been raised to 2.5 or later. 2.4 has
reached its end of life.

Running this benchmarking code:

``` ruby
Benchmark.ips do |x|
  x.report('empty') { Berns.element(:a) { 'Link to something' } }
  x.report('simple') { Berns.element(:a, { href: 'Something', class: 'my-class' }) { 'Link to something' } }
  x.report('nested') { Berns.element(:a, { href: 'Something', class: 'my-class', data: { something: 'Else' } }) { 'Link to something' } }

  x.compare!
end
```

Before:

``` example
 empty    993.521k (± 1.7%) i/s -      5.062M in   5.096368s
simple    340.795k (± 0.4%) i/s -      1.729M in   5.074101s
nested    215.160k (± 1.0%) i/s -      1.081M in   5.025324s
```

After:

``` example
 empty      1.769M (± 1.9%) i/s -      9.012M in   5.094973s
simple    441.020k (± 1.0%) i/s -      2.233M in   5.063326s
nested    280.255k (± 3.0%) i/s -      1.400M in   5.001009s
```

With empty attributes we see ~ 100% increase in iterations per second, with
simple attributes we see ~ 30% increase in the same, and with nested attributes
we see ~ 30% increase as well.

## 1.3.0

With version 1.3, nested HTML attributes can be created with nil keys and
boolean values to produce e.g. "data-foo data-foo-bar='whatever'" from `data: {
foo: { nil => true, bar: 'whatever' } }`

## 1.2.0 - 1.2.2

Starting with version 1.2, Berns will now HTML-escape all attribute values using
`CGI.escapeHTML`. This should prevent attribute values from escaping themselves
and injecting HTML into the DOM.

## 1.1.0

-   Add `#sanitize` method.
