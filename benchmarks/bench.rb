# frozen_string_literal: true
require 'bundler/inline'
require 'optparse'

options = {}

OptionParser.new do |parser|
  parser.banner = 'Usage: version.rb [options]'

  parser.on('--version VERSION', 'The version of Berns to benchmark e.g. "4.2.0" or "local" to benchmark the local copy') do |v|
    options[:version] = v
  end

  parser.on('-h', '--help', 'Prints this help') do
    puts parser
    exit
  end
end.parse!

gemfile do
  source 'https://rubygems.org'

  gem 'benchmark-ips', require: 'benchmark/ips'

  if options[:version] == 'local'
    gem 'berns', path: File.expand_path('../', __dir__)
  else
    gem 'berns', options[:version]
  end
end

puts '========'
puts "Berns #{ Berns::VERSION if Berns.const_defined?(:VERSION) } == #{ options[:version] }"

puts '========'
puts 'sanitize'

Benchmark.ips do |x|
  x.report('clean') { Berns.sanitize('This is a page title') }
  x.report('dirty') { Berns.sanitize('<span>This is a </span> page title') }

  x.compare!
end

puts '========'
puts 'to_attributes'

SIMPLE = { ryan: 'started the fire', itjust: 'started burning', hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' }.freeze
NESTED = { ryan: 'started the fire', itjust: { hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' } }.freeze
HUGE = (0..256).each_with_object({}) do |count, attrs|
  attrs["data-#{ count }"] = "This is data attribute number #{ count }"
end.freeze

Benchmark.ips do |x|
  x.report('simple') { Berns.to_attributes(SIMPLE) }
  x.report('nested') { Berns.to_attributes(NESTED) }
  x.report('huge') { Berns.to_attributes(HUGE) }

  x.compare!
end

puts '========'
puts 'to_attribute'

hash = { ryan: 'started the fire', itjust: 'started burning', hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' }
nested = { ryan: 'started the fire', itjust: { hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' } }

Benchmark.ips do |x|
  x.report('hash') { Berns.to_attribute(:data, hash) }
  x.report('nested hash') { Berns.to_attribute(:data, nested) }
  x.report('string') { Berns.to_attribute(:data, 'simple enough string') }
  x.report('bad string') { Berns.to_attribute(:data, 'simple <enough> "string"') }

  x.compare!
end

puts '========'
puts 'element'

simple = { href: 'Something', class: 'my-class' }
nested = { href: 'Something', class: 'my-class', data: { something: 'Else' } }

Benchmark.ips do |x|
  x.report('empty') { Berns.element(:a) { 'Link to something' } }
  x.report('simple') { Berns.element(:a, simple) { 'Link to something' } }
  x.report('nested') { Berns.element(:a, nested) { 'Link to something' } }

  x.compare!
end

if Berns.respond_to?(:escape_html)
  puts '========'
  puts 'escape_html'

  Benchmark.ips do |x|
    x.report('empty') { Berns.escape_html('') }
    x.report('clean') { Berns.escape_html("Forget the fat lady! You're obsessed with the fat lady! Drive us out of here! They're using our own satellites against us. And the clock is ticking. Life finds a way. My dad once told me, laugh and the world laughs with you, Cry, and I'll give you something to cry about you little bastard!") }
    x.report('dirty') { Berns.escape_html("<&'>\"") }

    x.compare!
  end
end
