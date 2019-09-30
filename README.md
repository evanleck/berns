# Berns

[![Gem Version](https://badge.fury.io/rb/berns.svg)](https://badge.fury.io/rb/berns)
[![Build Status](https://secure.travis-ci.org/evanleck/berns.svg)](https://travis-ci.org/evanleck/berns)

A utility library for generating HTML strings.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'berns'
```

And then execute:

```sh
bundle
```

Or install it yourself as:

```sh
gem install berns
```

## Usage

All standard and void HTML elements are defined as methods on Berns, so you can
create e.g. a link with `Berns.a`.

```rb
Berns.a(href: 'https://duckduckgo.com') { 'Search for things!' } # => "<a href=\"https://duckduckgo.com\">Search for things!</a>"
```

Nested attributes will become prefixes like so:

```rb
Berns.div(data: { controller: 'something->that#doesSomething' }) # => "<div data-controller=\"something->that#doesSomething\"></div>"
```

Lastly, all attribute values are HTML-escaped using `CGI.escapeHTML`.

## Development

After checking out the repo, run `bundle` to install dependencies. Then, run
`rake test` to run the tests.

To install this gem onto your local machine, run `bundle exec rake install`. To
release a new version, update the version number in the gemspec, and then run
`bundle exec rake release`, which will create a git tag for the version, push
git commits and tags, and push the `.gem` file to
[rubygems.org](https://rubygems.org).

## Contributing

Bug reports and pull requests are welcome on GitHub at
https://github.com/evanleck/berns. This project is intended to be a safe,
welcoming space for collaboration, and contributors are expected to adhere to
the [Contributor Covenant](http://contributor-covenant.org) code of conduct.

## License

The gem is available as open source under the terms of the [MIT
License](http://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the Berns project’s codebases, issue trackers, chat
rooms and mailing lists is expected to follow the [code of
conduct](https://github.com/evanleck/berns/blob/master/CODE_OF_CONDUCT.md).
