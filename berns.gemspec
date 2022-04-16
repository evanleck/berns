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
  spec.required_ruby_version = '>= 2.7.0'
  spec.required_rubygems_version = '>= 2.0'

  spec.files = Dir['ext/**/*', 'lib/**/*', 'README.org', 'LICENSE.txt']
  spec.require_paths = ['lib']
  spec.extensions = %w[ext/berns/extconf.rb]

  spec.metadata['bug_tracker_uri'] = 'https://github.com/evanleck/berns/issues'
  spec.metadata['changelog_uri'] = 'https://github.com/evanleck/berns/blob/main/CHANGELOG.org'
  spec.metadata['homepage_uri'] = spec.homepage
  spec.metadata['rubygems_mfa_required'] = 'true'
  spec.metadata['source_code_uri'] = 'https://github.com/evanleck/berns'
end
