# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'benchmark/ips'
require 'berns'
require 'cgi/escape'

EMPTY = ''
SPACE = ' '

def to_attributes(attributes)
  return EMPTY if attributes.empty?

  string = +''

  attributes.each do |attr, value|
    string << SPACE

    to_attribute(attr, value, string)
  end

  string.strip!
  string
end

def to_attribute(attribute, value, string = +'')
  if value.is_a?(FalseClass) # rubocop:disable Style/CaseLikeIf
  # noop
  elsif value.is_a?(TrueClass)
    string << attribute.to_s
  elsif value.is_a?(Hash)
    value.each do |attr, subval|
      string << SPACE

      to_attribute(attr.nil? ? attribute : "#{ attribute }-#{ attr }", subval, string)
    end

    string.strip!
  else
    string << %(#{ attribute }="#{ CGI.escapeHTML(value.to_s) }")
  end

  string
end

def element(tag, attributes = nil)
  content = yield if block_given?

  "<#{ tag } #{ to_attributes(attributes) }>#{ content }</#{ tag }>"
end

ATTRS = { class: 'class', data: { attr: 'value' } }.freeze

Benchmark.ips do |x|
  x.report('ruby')  { element('p', ATTRS) { 'Content' } }
  x.report('c-ext') { Berns.element('p', ATTRS) { 'Content' } }

  x.compare!
end
