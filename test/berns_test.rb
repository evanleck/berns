# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'berns'
require 'bigdecimal'
require 'minitest/autorun'

describe Berns do
  specify 'it has a version' do
    refute_nil ::Berns::VERSION
  end

  describe '#to_attribute' do
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

  # to_attributes is just a loop around to_attribute
  describe '#to_attributes' do
    it 'converts a single hash into attributes' do
      assert_equal 'this="tag"', Berns.to_attributes(this: 'tag')
      assert_equal 'this="tag" should="work" data-foo="bar" data-bar-baz="foo"', Berns.to_attributes(this: 'tag', should: 'work', data: { foo: 'bar', bar: { baz: 'foo' } })
    end

    it 'returns an empty string for empty attributes' do
      assert_equal '', Berns.to_attributes({})
    end

    it 'raises an error for non-hash values' do
      assert_raises(TypeError) { Berns.to_attributes(nil) }
      assert_raises(TypeError) { Berns.to_attributes([]) }
      assert_raises(TypeError) { Berns.to_attributes('nah') }
      assert_raises(TypeError) { Berns.to_attributes(:nope) }
    end

    it 'handles unicode characters fine' do
      assert_equal %(href="Working… on it"), Berns.to_attributes(href: 'Working… on it')
    end
  end

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

  describe '#void' do
    it 'generates void elements' do
      assert_equal '<br>', Berns.void('br')
      assert_equal '<br>', Berns.void(:br)
    end

    it 'generates void elements with attributes' do
      assert_equal '<br this="tag" should="work">', Berns.void('br', 'this' => 'tag', 'should' => 'work')
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

  describe '#element' do
    it 'is does not check the element type' do
      assert_equal '<br>ORLY</br>', Berns.element(:br) { 'ORLY' }
    end

    it 'creates empty standard elements' do
      assert_equal '<div></div>', Berns.element('div')
      assert_equal '<div></div>', Berns.element(:div)
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

  describe 'UTF-8 string encoding' do
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

  describe 'void element singleton methods' do
    %i[area base br col embed hr img input link menuitem meta param source track wbr].each do |void|
      it "has the void element ##{ void } method" do
        assert_equal "<#{ void }>", Berns.send(void)
        assert_equal %(<#{ void } class="my-area-element">), Berns.send(void, class: 'my-area-element')
        assert_equal "<#{ void }>", Berns.send(void) { 'Content' }
      end
    end
  end

  describe 'standard element singleton methods' do
    %i[a abbr address article aside audio b bdi bdo blockquote body button canvas caption cite code colgroup datalist dd del details dfn dialog div dl dt em fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i iframe ins kbd label legend li main map mark menu meter nav noscript object ol optgroup option output p picture pre progress q rp rt ruby s samp script section select small span strong style sub summary table tbody td template textarea tfoot th thead time title tr u ul var video].each do |standard|
      it "has the standard element ##{ standard } method" do
        assert_equal "<#{ standard }></#{ standard }>", Berns.send(standard)
        assert_equal "<#{ standard }>Something</#{ standard }>", (Berns.send(standard) { 'Something' })
        assert_equal %(<#{ standard } class="something">Something</#{ standard }>), (Berns.send(standard, class: 'something') { 'Something' })
      end
    end
  end

  describe '#sanitize' do
    it 'should remove HTML from strings' do
      assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean</span>')
      assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean')
      assert_nil Berns.sanitize(nil)
    end
  end
end
