# frozen_string_literal: true
require 'cgi/escape'

# A utility library for generating HTML strings
module Berns
  SPACE = ' '
  EMPTY = ''

  # Regular expression for basic HTML tag sanitizing.
  SANITIZE_REGEX = /<[^>]+>/.freeze

  # Full list of void elements - http://xahlee.info/js/html5_non-closing_tag.html
  VOID = %i[area base br col embed hr img input link menuitem meta param source track wbr].freeze

  # Full list of standard HTML5 elements - https://www.w3schools.com/TAgs/default.asp
  STANDARD = %i[a abbr address article aside audio b bdi bdo blockquote body button canvas caption cite code colgroup datalist dd del details dfn dialog div dl dt em fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i iframe ins kbd label legend li main map mark menu meter nav noscript object ol optgroup option output p picture pre progress q rp rt ruby s samp script section select small span strong style sub summary table tbody td template textarea tfoot th thead time title tr u ul var video].freeze

  # Dynamically defined methods that are simple proxies to {Markup#element}.
  STANDARD.each do |elm|
    define_singleton_method(elm) do |arguments = {}, &block|
      element(elm, arguments, &block)
    end
  end

  # Dynamically defined methods that are simple proxies to {Markup#void}.
  VOID.each do |elm|
    define_singleton_method(elm) do |arguments = {}|
      void(elm, arguments)
    end
  end

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

  # Generate a simple HTML element.
  #
  # @example Create an element with simple attributes.
  #   element(:a, href: '#nerds') { 'Nerds!'} # => "<a href='#nerds'>Nerds!</a>"
  #
  # @param tag [Symbol, String]
  #   The tag type to generate e.g. <a> or <script>.
  # @param attributes [Hash]
  #   A hash of attributes to add to the generated element.
  # @yieldreturn [String]
  #   The textual content of the element. May be HTML or plain text.
  # @return [String]
  def self.element(tag, attributes = {})
    content = yield if block_given?

    # Move stuff around unless the attributes are empty.
    attrs = to_attributes(attributes)
    attrs = " #{ attrs }" unless attrs.empty?

    "<#{ tag }#{ attrs }>#{ content }</#{ tag }>"
  end

  # Same as above, but generates void elements i.e. ones without any textual
  # content.
  #
  # @example Create a self-closing element.
  #   void(:br) # => "<br>"
  #
  # @return [String]
  def self.void(tag, attributes = {})
    # Move stuff around unless the attributes are empty.
    attrs = to_attributes(attributes)
    attrs = " #{ attrs }" unless attrs.empty?

    "<#{ tag }#{ attrs }>"
  end

  # Converts a hash into HTML attributes by mapping each key/value combination
  # to {#to_attribute} which actually does the hard work.
  #
  # @example A simple, single-level hash.
  #   to_attributes({ href: '#link' }) # => "href='#link'"
  #
  # @example A nested hash.
  #   to_attributes(href: '#nerds', some: { stuff: 'foobar' }) # => "href='#nerds' some-stuff='foobar'"
  #
  # @param attributes [Hash]
  #   The hash to convert to HTML attributes.
  # @return [String]
  #   The space-joined string containing HTML attributes.
  def self.to_attributes(attributes)
    attributes.map do |attribute, value|
      to_attribute(attribute, value)
    end.join(SPACE).chomp(SPACE)
  end

  # Converts a single attribute and value into an HTML attribute string.
  #
  # @example Obtain a boolean attribute string.
  #   to_attribute('nerf', true) # => 'nerf'
  #
  # @param attribute [#to_s]
  #   The attribute key.
  # @param value [String, Boolean, Hash]
  #   The value to assign to the attribute.
  # @return [String]
  #   A single HTML attribute.
  def self.to_attribute(attribute, value)
    case value
    when TrueClass
      attribute.to_s
    when Hash
      value.map do |subattribute, subvalue|
        "#{ attribute }-#{ to_attribute(subattribute, subvalue) }"
      end.join(SPACE)
    when FalseClass
      EMPTY
    else
      %(#{ attribute }="#{ CGI.escapeHTML(value.to_s) }")
    end
  end
end
