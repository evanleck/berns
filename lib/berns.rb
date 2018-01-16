# frozen_string_literal: true
require "berns/version"

module Berns
  SPACE = ' '
  EMPTY = ''

  # Full list of void elements - http://xahlee.info/js/html5_non-closing_tag.html
  VOID = %i[area base br col embed hr img input link menuitem meta param source track wbr]

  # Full list of standard HTML5 elements - https://www.w3schools.com/TAgs/default.asp
  STANDARD = %i[a abbr address article aside audio b bdi bdo blockquote body button canvas caption cite code colgroup datalist dd del details dfn dialog div dl dt em fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i iframe ins kbd label legend li main map mark menu meter nav noscript object ol optgroup option output p picture pre progress q rp rt ruby s samp script section select small span strong style sub summary table tbody td template textarea tfoot th thead time title tr u ul var video]

  VOID.each do |element|
    define_singleton_method(element) do |arguments = {}|
      # Move stuff around unless the attributes are empty.
      attrs = to_attributes(arguments)
      attrs = " #{ attrs }" unless attrs.empty?

      "<#{ element }#{ attrs }>"
    end
  end

  STANDARD.each do |elm|
    define_singleton_method(elm) do |arguments = {}, &content|
      Berns.element(elm, arguments, &content)
    end
  end

  # Generate a simple HTML5 element.
  #
  # @example Create an element with simple attributes.
  #   element(:a, href: '#nerds') { 'Nerds!'} # => "<a href='#nerds'>Nerds!</a>"
  #
  # @param tag [Symbol, String]
  #   The tag type to generate e.g. <a> or <script>.
  # @param attributes [Hash]
  #   A hash of attributes to add to the generated element.
  def self.element(tag, attributes = {}, &content)
    text = content ? content.call : EMPTY

    # Move stuff around unless the attributes are empty.
    attrs = to_attributes(attributes)
    attrs = " #{ attrs }" unless attrs.empty?

    "<#{ tag }#{ attrs }>#{ text }</#{ tag }>"
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
      "#{ attribute }='#{ value }'"
    end
  end
end
