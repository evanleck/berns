# Berns Changelog

## 2.0.0

Version 2.0 is a mostly API-compatible refactor of all of the core methods that
make up Berns. The goal is to improve performance, mostly using mutable strings
and inlining variables that were otherwise short lived.

In addition, the target Ruby version has been raised to 2.5 or later. 2.4 has
reached its end of life.

Running this benchmarking code:

```rb
Benchmark.ips do |x|
  x.report('empty') { Berns.element(:a) { 'Link to something' } }
  x.report('simple') { Berns.element(:a, { href: 'Something', class: 'my-class' }) { 'Link to something' } }
  x.report('nested') { Berns.element(:a, { href: 'Something', class: 'my-class', data: { something: 'Else' } }) { 'Link to something' } }

  x.compare!
end
```

Before:

```
 empty    993.521k (± 1.7%) i/s -      5.062M in   5.096368s
simple    340.795k (± 0.4%) i/s -      1.729M in   5.074101s
nested    215.160k (± 1.0%) i/s -      1.081M in   5.025324s
```

After:

```
 empty      1.769M (± 1.9%) i/s -      9.012M in   5.094973s
simple    441.020k (± 1.0%) i/s -      2.233M in   5.063326s
nested    280.255k (± 3.0%) i/s -      1.400M in   5.001009s
```

With empty attributes we see ~ 100% increase in iterations per second, with
simple attributes we see ~ 30% increase in the same, and with nested attributes
we see ~ 30% increase as well.

## 1.3.0

With version 1.3, nested HTML attributes can be created with nil keys and
boolean values to produce e.g. "data-foo data-foo-bar='whatever'" from
`data: { foo: { nil => true, bar: 'whatever' } }`

## 1.2.0 - 1.2.2

Starting with version 1.2, Berns will now HTML-escape all attribute values using
`CGI.escapeHTML`. This should prevent attribute values from escaping themselves
and injecting HTML into the DOM.

## 1.1.0

- Add `#sanitize` method.
