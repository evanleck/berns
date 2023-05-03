# Berns

[![](https://badge.fury.io/rb/berns.svg)](https://badge.fury.io/rb/berns)
[![](https://github.com/evanleck/berns/actions/workflows/main.yml/badge.svg)](https://github.com/evanleck/berns/actions/workflows/main.yml)

A utility library for generating HTML strings.

## Installation

Add this line to your application's Gemfile:

``` ruby
gem 'berns'
```

And then execute:

``` sh
bundle
```

Or install it yourself as:

``` sh
gem install berns
```

## Usage

Note that all return string values will be UTF-8 encoded.

### `void(tag, attributes)`

The `void` method generates a void HTML element i.e. one without any content. It
takes either a symbol or a string and an optional hash of HTML attributes.

``` ruby
Berns.void('br') # => '<br>'
Berns.void('br', class: 'br-class') # => '<br class="br-class">'
```

### `element(tag, attributes) { content }`

The `element` method generates a standard HTML element i.e. one with optional
content. It takes either a symbol or a string, an optional hash of HTML
attributes, and an optional block of content. If provided, the block should
return a string.

``` ruby
Berns.element('div') # => '<div></div>'
Berns.element('div', class: 'div-class') # => '<div class="div-class"></div>'
Berns.element('div', class: 'div-class') { 'Content' } # => '<div class="div-class">Content</div>'
```

### `to_attribute(attribute, value)`

The `to_attribute` method generates an HTML attribute string. If the value is a
hash then the attribute is treated as a prefix.

``` ruby
Berns.to_attribute('class', 'my-class another-class') # => 'class="my-class another-class"'
Berns.to_attribute('data', { foo: 'bar' }) # => 'data-foo="bar"'
```

All attribute values are HTML-escaped using [k0kubun/hescape](hescape) written
by Takashi Kokubun.

### `to_attributes(attributes)`

The `to_attributes` method generates an HTML attribute string from a hash by
calling `to_attribute` for each key/value pair.

``` ruby
Berns.to_attributes({ 'data' => { foo: 'bar' }, 'class' => 'my-class another-class' }) # => 'data-foo="bar" class="my-class another-class"'
```

### `escape_html(string)`

The `escape_html` method escapes HTML entities in strings using
[k0kubun/hescape](hescape) written by Takashi Kokubun. As noted in the hescape
repository, it should be the same as `CGI.escapeHTML`.

``` ruby
Berns.escape_html('<"tag"') # => '&lt;&quot;tag&quot;'
```

### `sanitize(string)`

The `sanitize` method strips HTML tags from strings.

``` ruby
Berns.sanitize('This <span>should be clean</span>') # => 'This should be clean'
```

Note that this is an extremely naive implementation of HTML sanitization that
literally just looks for "<" and ">" characters and removes the contents between
them. This should probably only be used on trusted strings.

### `build { content }`

The `build` method uses `Berns::Builder` to let you create HTML strings using a
DSL.

``` ruby
Berns.build { h1 { 'Heading' } } # => '<h1>Heading</h1>'
```

See below for more on `Berns::Builder`.

### `Berns::Builder` HTML DSL

Added in version 3.4.0 and heavily inspired by the likes of
[Papercraft](papercraft), [Markaby](markaby), and [Arbre](arbre), the
`Berns::Builder` class lets you create HTML strings using a DSL.

``` ruby
template = Berns::Builder.new do
  h1 { 'Heading' }
  p(class: 'paragraph') do
    text 'Bare text here.'

    b { 'Bold text here' }
  end
end
```

Within the block provided to `Berns::Builder.new` every standard element method,
void element method, `#element`, and `#void` are available as methods and each
time you use one of those methods the result is appended to an internal buffer.
In addition, the `#text` method appends HTML escaped text to the buffer and
`#raw` appends text to the buffer without modification.

The block provided to `Berns::Builder.new` can take both positional and keyword
arguments.

``` ruby
template = Berns::Builder.new do |content, title:|
  h1 { title }
  p(class: 'paragraph') { content }
end

template.call('Some text.', title: 'The title') # =>
# <h1>
#   The title
# </h1>
# <p>
#   Some text.
# </p>
```

Once initialized, the `#call` method will render the template to a string. Any
arguments, positional or keyword, are passed through as-is to the block provided
to `#new`.

``` ruby
string = template.call # =>
# <h1>
#   Heading
# </h1>
# <p class='paragraph'>
#   Bare text here.
#   <b>
#     Bold text here.
#   </b>
# </p>
```

In addition to initializing a new instance of `Berns::Builder`, you can
construct and render a template to a string all at once with `Berns.build`.

``` ruby
Berns.build do
  h1 { 'Heading' }
  p(class: 'paragraph') do
    text 'Bare text here.'

    b { 'Bold text here' }
  end
end # =>
# <h1>
#   Heading
# </h1>
# <p class='paragraph'>
#   Bare text here.
#   <b>
#     Bold text here.
#   </b>
# </p>
```

### Standard and void elements

All standard and void HTML elements are defined as methods on Berns, so you can
create e.g. a link with `Berns.a`. Below is the full list of standard elements
which are also available in the constant `Berns::STANDARD` as an array of
symbols.

```
a abbr address article aside audio b bdi bdo blockquote body button canvas
caption cite code colgroup datalist dd del details dfn dialog div dl dt em
fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i
iframe ins kbd label legend li main map mark menu meter nav noscript object ol
optgroup option output p picture pre progress q rp rt ruby s samp script section
select small span strong style sub summary table tbody td template textarea
tfoot th thead time title tr u ul var video
```

Below is the full list of void elements that are defined as singleton methods on
Berns which are also available in the constant `Berns::VOID` as an array of
symbols.

```
area base br col embed hr img input link menuitem meta param source track wbr
```

## Performance

Berns 3 was a total rewrite from a pure Ruby implementation to one powered by a
C extension. That rewrite is about three times faster than the pure Ruby
implementation used in version 2. See the file
[benchmarks/performance.rb](benchmarks/performance.rb) for the benchmark code.

``` example
Warming up --------------------------------------
                ruby    27.521k i/100ms
               c-ext    74.915k i/100ms
Calculating -------------------------------------
                ruby    275.913k (± 1.0%) i/s -      1.404M in   5.087516s
               c-ext    813.113k (± 1.0%) i/s -      4.120M in   5.067902s

Comparison:
               c-ext:   813113.3 i/s
                ruby:   275913.5 i/s - 2.95x  (± 0.00) slower
```

## Trivia

The name "Berns" is taken from the name of [the inventor of HTML](html), [Sir
Tim Berners-Lee](tim).

<!-- Links -->
[arbre]: https://github.com/activeadmin/arbre
[hescape]: https://github.com/k0kubun/hescape
[html]: https://en.wikipedia.org/wiki/HTML#Development
[markaby]: https://github.com/markaby/markaby
[papercraft]: https://github.com/digital-fabric/papercraft
[tim]: https://en.wikipedia.org/wiki/Tim_Berners-Lee
