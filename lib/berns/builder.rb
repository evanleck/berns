# frozen_string_literal: true
require 'berns'

module Berns
  # An HTML builder DSL using Berns' HTML methods.
  class Builder
    def initialize(&block)
      @block = block
      @buffer = +''
    end

    # @return [String]
    def call(*args, **opts)
      instance_exec(*args, **opts, &@block)
      to_s
    end

    # @return [String]
    def to_s
      @buffer.freeze
    end

    # Append text to the buffer.
    #
    # @param string [String]
    # @return [String]
    def text(string)
      @buffer << Berns.escape_html(string.to_s)
    end

    Berns::STANDARD.each do |meth|
      define_method(meth) do |*args, **opts, &block|
        content = Builder.new.instance_exec(*args, **opts, &block) if block
        @buffer << Berns.send(meth, *args, **opts) { content }
      end
    end

    Berns::VOID.each do |meth|
      define_method(meth) do |*args, **opts|
        @buffer << Berns.send(meth, *args, **opts)
      end
    end
  end
end
