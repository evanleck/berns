# frozen_string_literal: true
require_relative 'lib/berns'

Gem::Specification.new do |spec|
  spec.name = 'berns'
  spec.version = Berns::VERSION
  spec.authors = ['Taylor Beck']
  spec.email = ['taylor.beck@engageft.com']

  spec.summary = 'A utility library for generating HTML strings.'
  spec.description = 'A utility library for generating HTML strings.'
  spec.homepage = 'https://github.com/TaylorBeck/berns'
  spec.license = 'MIT'

  spec.files = ['lib/berns.rb']
  spec.require_paths = ['lib']

  spec.add_development_dependency 'bundler'
  spec.add_development_dependency 'minitest'
  spec.add_development_dependency 'rake'
end
