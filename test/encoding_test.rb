# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe 'Berns string encoding' do
  it 'returns only UTF-8 strings' do
    assert_equal Encoding::UTF_8, Berns.element('div').encoding
    assert_equal Encoding::UTF_8, Berns.void('br').encoding
    assert_equal Encoding::UTF_8, Berns.to_attributes(href: { stuff: { another: 'foobar' }, blerg: 'Flerr' }).encoding
  end

  it 'converts to UTF-8' do
    ascii = (+'This is an ASCII…string').force_encoding(Encoding::US_ASCII)
    result = Berns.to_attributes(href: ascii)

    assert_equal Encoding::UTF_8, result.encoding
    assert_equal %(href="This is an ASCII…string"), result
  end
end
