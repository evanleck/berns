# frozen_string_literal: true
require 'berns'
require 'bigdecimal'
require 'minitest/autorun'

describe 'Berns#element' do
  it 'is does not check the element type' do
    assert_equal '<br>ORLY</br>', Berns.element(:br) { 'ORLY' }
  end

  it 'creates empty standard elements' do
    assert_equal '<div></div>', Berns.element('div')
    assert_equal '<div></div>', Berns.element(:div)
  end

  it 'creates empty standard elements with attributes' do
    assert_equal '<div></div>', Berns.element('div', {})
    assert_equal '<div></div>', Berns.element(:div, {})
    assert_equal '<div one="One"></div>', Berns.element('div', { one: 'One' })
    assert_equal '<div one="One" two="Two"></div>', Berns.element('div', { one: 'One', two: 'Two' })
  end

  it 'creates standard elements with a block of content' do
    assert_equal '<div>Content</div>', Berns.element('div') { 'Content' }
    assert_equal '<div></div>', Berns.element('div') { nil }
    assert_equal '<div></div>', Berns.element('div') { false }
    assert_equal '<div>3</div>', Berns.element('div') { 3 }
    assert_equal '<div>0.3e1</div>', Berns.element('div') { BigDecimal('3.0') }
    assert_equal '<div>["one", "or", "another"]</div>', Berns.element('div') { %w[one or another] }
    assert_equal '<div>{"one"=>"oranother"}</div>', Berns.element('div') { { 'one' => 'oranother' } }
  end

  it 'raises an error for non-hash second arguments' do
    assert_raises(TypeError) { Berns.element('hr', 'what this') }
    assert_raises(TypeError) { Berns.element('hr', 2) }
    assert_raises(TypeError) { Berns.element('hr', :nope) }
    assert_raises(TypeError) { Berns.element('hr', ['hey']) }
    assert_raises(TypeError) { Berns.element('hr', nil) }
  end
end
