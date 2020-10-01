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
