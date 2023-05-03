# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe 'Berns#void' do
  it 'generates void elements' do
    assert_equal '<br>', Berns.void('br')
    assert_equal '<br>', Berns.void(:br)
  end

  it 'generates void elements with empty attributes' do
    assert_equal '<br>', Berns.void(:br, {})
  end

  it 'generates void elements with attributes' do
    assert_equal '<br this="tag">', Berns.void('br', 'this' => 'tag')
    assert_equal '<br this="tag" should="work">', Berns.void('br', 'this' => 'tag', 'should' => 'work')
    assert_equal '<br this="tag">', Berns.void('br', this: 'tag')
    assert_equal '<br this="tag" should="work">', Berns.void('br', this: 'tag', should: 'work')
    assert_equal '<br this="tag" should="work" data-foo="bar" data-bar-baz="foo">', Berns.void('br', this: 'tag', should: 'work', data: { foo: 'bar', bar: { baz: 'foo' } })
  end

  it 'ignores content blocks' do
    assert_equal('<hr>', Berns.void('hr') { 'Content' })
  end

  it 'raises an error for non-hash second arguments' do
    assert_raises(TypeError) { Berns.void('hr', 'what this') }
    assert_raises(TypeError) { Berns.void('hr', 2) }
    assert_raises(TypeError) { Berns.void('hr', :nope) }
    assert_raises(TypeError) { Berns.void('hr', ['hey']) }
    assert_raises(TypeError) { Berns.void('hr', nil) }
  end
end
