# frozen_string_literal: true
require_relative 'lib/berns/version'

Gem::Specification.new do |spec|
  spec.name = 'berns'
  spec.version = Berns::VERSION
  spec.authors = ['Taylor Beck', 'Evan Lecklider']
  spec.email = ['beck.taylorg@gmail.com', 'evan@lecklider.com']

  spec.summary = 'A utility library for generating HTML strings.'
  spec.description = spec.summary
  spec.homepage = 'https://github.com/evanleck/berns'
  spec.license = 'MIT'
  spec.required_ruby_version = Gem::Requirement.new('>= 2.5.0')

  spec.files = Dir['ext/**/*', 'lib/**/*', 'README.org', 'LICENSE.txt']
  spec.require_paths = ['lib']
  spec.extensions = %w[ext/berns/extconf.rb]

  spec.metadata = {
    'bug_tracker_uri' => 'https://github.com/evanleck/berns/issues',
    'source_code_uri' => 'https://github.com/evanleck/berns'
  }

  spec.add_dependency 'cgi'

  spec.add_development_dependency 'bundler'
  spec.add_development_dependency 'minitest'
  spec.add_development_dependency 'rake'
  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency 'rubocop'
  spec.add_development_dependency 'rubocop-minitest'
  spec.add_development_dependency 'rubocop-packaging'
  spec.add_development_dependency 'rubocop-performance'
  spec.add_development_dependency 'rubocop-rake'
end
