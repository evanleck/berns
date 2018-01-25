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

Simply use the `Berns::[element]` method to generate and return HTML5 elements as strings by any attributes, their values, and  the text/content of the element.

__NOTE:__ Berns can generate elements with ([standard](https://www.w3schools.com/TAgs/default.asp)) and without ([void](http://xahlee.info/js/html5_non-closing_tag.html)) closing tags.

### Examples

__Create an anchor element with one attribute.__

`a(href: '#target') { 'Click Me!'}`

`# => "<a href='#target'>Click Me!</a>"`

__Create a link element with multiple attributes.__

`link(rel: 'stylesheet', href: '/path/to/style', type: 'text/css')`

`# => "<link rel='stylesheet' href='/path/to/style' type='text/css'>"`

__Create an img element with a source and alt text.__

`img(src: '/path/to/image', alt: 'An image')`

`# => "<img src='/path/to/image' alt='An image'>"`

__You can even accomplish more complicated HTML strings, such as an unordered list of dynamic data.__
```ruby
groceries = ['Apples', 'Milk', 'Eggs', 'Cereal', 'Juice']

ul(id: 'list-id', class: 'list-class') { groceries.map { |grocery_item| li(class: 'item-class') { grocery_item } }.join }

# => <ul id='list-id' class='list-class'><li class='item-class'>Apples</li><li class='item-class'>Milk</li><li class='item-class'>Eggs</li><li class='item-class'>Cereal</li><li class='item-class'>Juice</li></ul>
```

__NOTE:__ *`DOCTYPE` and `<!--...-->` are not currently supported.*

## License

The gem is available as open source under the terms of the [MIT License](http://opensource.org/licenses/MIT).

*Berns is named in recognition of the inventor of HTML, [Tim Berners-Lee](https://www.w3.org/People/Berners-Lee/).*
