# Berns

Berns is a simple Ruby utility library for generating HTML5 elements.

## Installing Berns

Add this line to your application's Gemfile:

```ruby
gem 'berns'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install berns

## Using Berns

Simply use the `Berns::element` method to generate and return HTML5 elements as strings by passing the tag type, any attributes and their values, and (optionally) the text/content of the element. 

`element(tag, attributes = {}, &content)`

__NOTE:__ The `element` method can generate elements with ([standard](https://www.w3schools.com/TAgs/default.asp)) and without ([void](http://xahlee.info/js/html5_non-closing_tag.html)) closing tags. 

### Examples
__Create an anchor element with simple attributes.__
`element(:a, href: '#target') { 'Click Me!'} # => "<a href='#target'>Click Me!</a>"`

__Create an img element with a source and alt text.__
`element(:img, src: '/path/to/image', alt: 'An image') # => <img src='/path/to/image' alt='An image'>`

__NOTE:__ `DOCTYPE` and `<!--...-->` are not currently supported.
## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).

*Berns is named in recognition of the inventor of HTML, [Tim Berners-Lee](https://www.w3.org/People/Berners-Lee/).*
