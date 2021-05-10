# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'benchmark/ips'
require 'berns'

puts 'to_attributes'
Benchmark.ips do |x|
  x.report('simple') { Berns.to_attributes({ ryan: 'started the fire', itjust: 'started burning', hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' }) }
  x.report('nested') { Berns.to_attributes({ ryan: 'started the fire', itjust: { hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' } }) }

  x.compare!
end

puts 'to_attribute'
Benchmark.ips do |x|
  x.report('hash') { Berns.to_attribute(:data, { ryan: 'started the fire', itjust: 'started burning', hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' }) }
  x.report('nested hash') { Berns.to_attribute(:data, { ryan: 'started the fire', itjust: { hey: "the temp's still learning", this: 'is a super long key that will just keep going on and on and on', more: 'keys are required to trigger a realloc' } }) }
  x.report('string') { Berns.to_attribute(:data, 'simple enough string') }
  x.report('bad string') { Berns.to_attribute(:data, 'simple <enough> "string"') }

  x.compare!
end

if Berns.respond_to?(:escape_html)
  puts 'escape_html'
  Benchmark.ips do |x|
    x.report('empty') { Berns.escape_html('') }
    x.report('clean') { Berns.escape_html("Forget the fat lady! You're obsessed with the fat lady! Drive us out of here! They're using our own satellites against us. And the clock is ticking. Life finds a way. My dad once told me, laugh and the world laughs with you, Cry, and I'll give you something to cry about you little bastard!") }
    x.report('dirty') { Berns.escape_html("<&'>\"") }

    x.compare!
  end
end

puts 'element'
Benchmark.ips do |x|
  x.report('empty') { Berns.element(:a) { 'Link to something' } }
  x.report('simple') { Berns.element(:a, { href: 'Something', class: 'my-class' }) { 'Link to something' } }
  x.report('nested') { Berns.element(:a, { href: 'Something', class: 'my-class', data: { something: 'Else' } }) { 'Link to something' } }

  x.compare!
end
