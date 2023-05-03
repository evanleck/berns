# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe Berns do
  describe '#escape_html' do
    it 'escapes HTML strings' do
      assert_equal '&lt;&quot;tag&quot;', Berns.escape_html('<"tag"')
    end

    it 'raises an error for non-string values' do
      assert_raises(TypeError) { Berns.escape_html(:nope) }
      assert_raises(TypeError) { Berns.escape_html(['nope']) }
      assert_raises(TypeError) { Berns.escape_html({ no: 'pe' }) }
    end
  end
end
