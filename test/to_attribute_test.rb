# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe 'Berns#to_attribute' do
  it 'handles boolean attribute values' do
    assert_equal 'nerf', Berns.to_attribute('nerf', true)
    assert_equal '', Berns.to_attribute('nerf', false)
  end

  it 'handles string attributes' do
    big = 'You know what? It is beets. Ive crashed into a beet truck. Did he just throw my cat out of the window? Hey, take a look at the earthlings. Goodbye! God help us, were in the hands of engineers. My dad once told me, laugh and the world laughs with you, Cry, and Ill give you something to cry about you little bastard!'

    assert_equal %(nerf="guns"), Berns.to_attribute('nerf', 'guns')
    assert_equal %(nerf="#{ big }"), Berns.to_attribute('nerf', big)
  end

  it 'handles symbol arguments' do
    assert_equal %(nerf="guns"), Berns.to_attribute(:nerf, :guns)
  end

  it 'handles non-string attributes' do
    assert_equal %(nerf-toy="guns"), Berns.to_attribute('nerf', 'toy' => 'guns')
    assert_equal %(foo="4"), Berns.to_attribute('foo', 4)
    assert_equal %(foo="bar"), Berns.to_attribute('foo', :bar)
    assert_equal %(foo="[&quot;bar&quot;]"), Berns.to_attribute('foo', ['bar'])
  end

  it 'does not rewrite underscores in attribute names' do
    assert_equal %(data-foo_bar="baz"), Berns.to_attribute('data', 'foo_bar' => 'baz')
  end

  it 'escapes the attribute content' do
    assert_equal %(foo="&lt;b&gt;bar&lt;/b&gt;-&quot;with &#39;quotes&#39;&quot;"), Berns.to_attribute('foo', %(<b>bar</b>-"with 'quotes'"))
  end

  it 'does not escape the attribute name itself' do
    assert_equal %(<b>foo="&lt;b&gt;bar&lt;/b&gt;-&quot;with &#39;quotes&#39;&quot;"), Berns.to_attribute('<b>foo', %(<b>bar</b>-"with 'quotes'"))
  end

  it 'handles flat hashes' do
    assert_equal %(data-foo="bar"), Berns.to_attribute('data', { 'foo' => 'bar' })
    assert_equal %(data-foo="bar" data-baz="foo"), Berns.to_attribute('data', { 'foo' => 'bar', 'baz' => 'foo' })
  end

  it 'handles big values' do
    big = 'You know what? It is beets. Ive crashed into a beet truck. Did he just throw my cat out of the window? Hey, take a look at the earthlings. Goodbye! God help us, were in the hands of engineers. My dad once told me, laugh and the world laughs with you, Cry, and Ill give you something to cry about you little bastard!'

    assert_equal %(data-foo="#{ big }" data-baz="#{ big }"), Berns.to_attribute('data', { 'foo' => big, 'baz' => big })
  end

  it 'nests hash values as key prefixes' do
    assert_equal 'data-foo="bar" data-bar-baz="foo" data-bar-zoo-keeper="Bob"', Berns.to_attribute(:data, { foo: 'bar', bar: { baz: 'foo', zoo: { keeper: 'Bob' } } })
    assert_equal %(data-something data-something-another="Foo"), Berns.to_attribute(:data, { something: { nil => true, another: 'Foo' } })
  end

  it 'returns the attribute name for true values' do
    assert_equal 'required', Berns.to_attribute('', { required: true })
  end

  it 'drops the attribute name for false values' do
    assert_equal '', Berns.to_attribute('', { required: false })
  end

  it 'handles unicode characters fine' do
    assert_equal %(href="Working… on it"), Berns.to_attribute(:href, 'Working… on it')
  end

  it 'returns an empty string for empty attributes' do
    assert_equal '', Berns.to_attribute(:data, {})
    assert_equal '', Berns.to_attribute(:data, { more: {} })
  end

  it 'handles empty attribute names' do
    assert_equal %(foo="bar"), Berns.to_attribute('', { foo: 'bar' })
  end

  it 'allows string, symbol, and nil type sub-attribute keys' do
    assert_equal 'data-foo', Berns.to_attribute(:data, { foo: nil })
    assert_equal 'data-foo="bar"', Berns.to_attribute(:data, { 'foo' => 'bar' })
    assert_equal 'data', Berns.to_attribute(:data, { nil => true })

    assert_raises(TypeError) { Berns.to_attribute(:data, { [] => true }) }
    assert_raises(TypeError) { Berns.to_attribute(:data, { {} => true }) }
    assert_raises(TypeError) { Berns.to_attribute(:data, { 2 => ['two'] }) }
  end

  it 'raises an error for non-string and symbol attribute names' do
    assert_raises(TypeError) { Berns.to_attribute(22, 'yeah') }
    assert_raises(TypeError) { Berns.to_attribute([], 'yeah') }
    assert_raises(TypeError) { Berns.to_attribute(nil, 'yeah') }
    assert_raises(TypeError) { Berns.to_attribute({}, 'yeah') }
  end
end
