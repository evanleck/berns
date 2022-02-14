# frozen_string_literal: true
require 'berns/berns'
require 'berns/version'

module Berns # :nodoc:
  autoload :Builder, 'berns/builder'

  STANDARD = %i[
    a abbr address article aside audio b bdi bdo blockquote body button canvas
    caption cite code colgroup datalist dd del details dfn dialog div dl dt em
    fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i
    iframe ins kbd label legend li main map mark menu meter nav noscript object
    ol optgroup option output p picture pre progress q rp rt ruby s samp script
    section select small span strong style sub summary table tbody td template
    textarea tfoot th thead time title tr u ul var video
  ].freeze

  VOID = %i[
    area base br col embed hr img input link menuitem meta param source track wbr
  ].freeze

  # @return [String]
  def self.build(*args, **opts, &block)
    Builder.new(&block).call(*args, **opts)
  end
end
