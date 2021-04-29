# frozen_string_literal: true
require 'berns/berns'
require 'berns/version'

module Berns # :nodoc:
  class Error < StandardError; end

  EMPTY = ''

  # Regular expression for basic HTML tag sanitizing.
  SANITIZE_REGEX = /<[^>]+>/.freeze

  # Sanitize text input by stripping HTML tags.
  #
  # @example Sanitize some text, removing HTML elements.
  #   sanitize('This <span>should be clean</span>') # => "This should be clean"
  #
  # @param text [String]
  #   The string to sanitize.
  # @return [nil, String]
  #   nil unless a string was passed in, otherwise the sanitized string.
  def self.sanitize(string)
    string&.gsub(SANITIZE_REGEX, EMPTY)
  end
end
