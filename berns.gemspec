# coding: utf-8
lib = File.expand_path("../lib", __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require "berns/version"

Gem::Specification.new do |spec|
  spec.name          = "berns"
  spec.version       = Berns::VERSION
  spec.authors       = ["Taylor Beck"]
  spec.email         = ["taylor.beck@engageft.com"]

  spec.summary       = %q{Utility library for generating HTML strings}
  spec.description   = %q{Utility library for generating HTML strings}
  spec.homepage      = "https://github.com/TaylorBeck/berns"
  spec.license       = "MIT"

  spec.files         = `git ls-files -z`.split("\x0").reject do |f|
    f.match(%r{^(test|spec|features)/})
  end
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]

  spec.add_development_dependency "bundler", "~> 1.15"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "minitest", "~> 5.0"
end
