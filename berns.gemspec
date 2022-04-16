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
  spec.required_ruby_version = '>= 2.5.0'
  spec.required_rubygems_version = '>= 2.0'

  spec.files = Dir['ext/**/*', 'lib/**/*', 'README.org', 'LICENSE.txt']
  spec.require_paths = ['lib']
  spec.extensions = %w[ext/berns/extconf.rb]

  spec.metadata['bug_tracker_uri'] = 'https://github.com/evanleck/berns/issues'
  spec.metadata['changelog_uri'] = 'https://github.com/evanleck/berns/blob/main/CHANGELOG.org'
  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['rubygems_mfa_required'] = 'true'
  spec.metadata['source_code_uri'] = 'https://github.com/evanleck/berns'

  spec.add_development_dependency 'benchmark-ips', '~> 2'
  spec.add_development_dependency 'minitest', '~> 5'
  spec.add_development_dependency 'rake', '~> 13'
  spec.add_development_dependency 'rake-compiler', '~> 1'
  spec.add_development_dependency 'rubocop', '~> 1'
  spec.add_development_dependency 'rubocop-minitest'
  spec.add_development_dependency 'rubocop-packaging'
  spec.add_development_dependency 'rubocop-performance'
  spec.add_development_dependency 'rubocop-rake'
end
