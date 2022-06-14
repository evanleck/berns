# frozen_string_literal: true
require 'berns'

module Berns
  # An HTML builder DSL using Berns' HTML methods.
  class Builder
    def initialize(&block)
      raise(ArgumentError, 'Berns::Builder initialized without a block argument', caller) unless block

      @block = block
    end

    # @return [String]
    def call(*args, **kwargs)
      @buffer = +''
      content = instance_exec(*args, **kwargs, &@block)

      # This is a special case where the buffer hasn't been appended to but the
      # block returned a string.
      if @buffer.empty? && content.is_a?(String)
        Berns.escape_html(content).freeze
      else
        @buffer.freeze
      end
    end
    alias to_s call
    alias to_str call

    # Append HTML escaped text to the buffer.
    #
    # @param string [String]
    # @return [String]
    def text(string)
      @buffer << Berns.escape_html(string.to_s)
    end

    # Append raw text to the buffer.
    #
    # @param string [String]
    # @return [String]
    def raw(string)
      @buffer << string.to_s
    end

    # Append an arbitrary standard element to the buffer.
    #
    # @return [String]
    def element(elm, *args, **kwargs, &block)
      content = Builder.new(&block).call if block
      @buffer << Berns.element(elm, *args, **kwargs) { content }
    end

    # Append an arbitrary void element to the buffer.
    #
    # @return [String]
    def void(...)
      @buffer << Berns.void(...)
    end

    Berns::STANDARD.each do |meth|
      define_method(meth) do |*args, **kwargs, &block|
        content = Builder.new(&block).call if block
        @buffer << Berns.send(meth, *args, **kwargs) { content }
      end
    end

    Berns::VOID.each do |meth|
      define_method(meth) do |*args, **kwargs|
        @buffer << Berns.send(meth, *args, **kwargs)
      end
    end
  end
end
