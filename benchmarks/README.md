# Benchmarks

These are contrived examples, but I've found it instructive to check results
over time. The benchmarks can be run from the project directory with `ruby
benchmarks/bench.rb --version VERSION` where `VERSION` can be a released gem
version or `local` to test the current working version.

Note that the sections labeled `Calculating` is the only one we really care
about across versions and the `Comparison` section is completely useless because
you're just comparing different arguments to the same function call.

My general workflow is:

1. Make whatever changes you'd like to make.
2. Run the benchmarks against those changes e.g. `ruby benchmarks/bench.rb
   --version local`.
3. Run the benchmarks against the latest release version e.g. `ruby
   benchmarks/bench.rb --version 4.2.0`.
4. Compare the `Calculating` sections from each set of results.

## v3.1.0

The performance in this release compared to the previous version, v3.0.6, is
effectively flat, which is fine. Compared to the previous major version, v2.0.0,
performance has approximately quadrupled, which is pretty cool.

``` example
to_attributes
Calculating -------------------------------------
              simple      1.204M (± 1.3%) i/s -      6.111M in   5.077247s
              nested    941.180k (± 1.9%) i/s -      4.745M in   5.043321s

to_attribute
Calculating -------------------------------------
                hash      1.128M (± 1.1%) i/s -      5.684M in   5.038439s
         nested hash    867.197k (± 1.1%) i/s -      4.359M in   5.026957s
              string      4.728M (± 0.5%) i/s -     23.812M in   5.036424s
          bad string      3.443M (± 1.5%) i/s -     17.533M in   5.093006s

escape_html
Calculating -------------------------------------
               empty     18.713M (± 1.6%) i/s -     93.814M in   5.014644s
               clean      3.236M (± 0.4%) i/s -     16.206M in   5.008352s
               dirty      3.968M (± 1.4%) i/s -     19.925M in   5.021970s

element
Calculating -------------------------------------
               empty      3.600M (± 1.3%) i/s -     18.122M in   5.034973s
              simple      1.713M (± 2.2%) i/s -      8.665M in   5.060961s
              nested      1.027M (± 1.8%) i/s -      5.139M in   5.006588s
```

## v3.0.6

``` example
to_attributes
Calculating -------------------------------------
              simple      1.260M (± 0.6%) i/s -      6.348M in   5.036953s
              nested    931.178k (± 2.2%) i/s -      4.698M in   5.047333s

to_attribute
Calculating -------------------------------------
                hash      1.114M (± 1.0%) i/s -      5.580M in   5.010324s
         nested hash    875.186k (± 0.6%) i/s -      4.417M in   5.047462s
              string      4.786M (± 0.6%) i/s -     24.062M in   5.027836s
          bad string      3.463M (± 0.7%) i/s -     17.559M in   5.070078s

escape_html
Calculating -------------------------------------
               empty     18.618M (± 1.3%) i/s -     94.785M in   5.091929s
               clean      3.274M (± 0.5%) i/s -     16.677M in   5.093523s
               dirty      4.020M (± 1.4%) i/s -     20.142M in   5.010843s

element
Calculating -------------------------------------
               empty      3.682M (± 1.8%) i/s -     18.458M in   5.014521s
              simple      1.773M (± 0.6%) i/s -      8.961M in   5.055727s
              nested      1.069M (± 0.6%) i/s -      5.369M in   5.023035s
```

## v2.0.0

``` example
to_attributes
Calculating -------------------------------------
              simple    239.926k (± 1.3%) i/s -      1.213M in   5.058548s
              nested    210.350k (± 1.5%) i/s -      1.063M in   5.052986s

to_attribute
Calculating -------------------------------------
                hash    179.166k (± 0.7%) i/s -    910.900k in   5.084381s
         nested hash    187.729k (± 0.5%) i/s -    945.750k in   5.037992s
              string      1.515M (± 0.5%) i/s -      7.664M in   5.057512s
          bad string      1.214M (± 0.5%) i/s -      6.122M in   5.044293s

element
Calculating -------------------------------------
               empty      1.643M (± 0.6%) i/s -      8.295M in   5.047921s
              simple    447.341k (± 0.6%) i/s -      2.257M in   5.045563s
              nested    268.844k (± 1.5%) i/s -      1.352M in   5.031703s
```
