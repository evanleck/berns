# frozen_string_literal: true
$LOAD_PATH.unshift File.expand_path('../lib', __dir__)

require 'berns'
require 'minitest/autorun'

class BernsTest < Minitest::Test
  def test_that_it_has_a_version_number
    refute_nil ::Berns::VERSION
  end

  def test_void_element
    assert_equal '<br>', Berns.void('br')
  end

  def test_void_element_from_symbol
    assert_equal '<br>', Berns.void(:br)
  end

  def test_void_element_with_attributes
    assert_equal '<br this="tag" should="work">', Berns.void('br', 'this' => 'tag', 'should' => 'work')
  end

  def test_void_element_with_symbol_attribute_keys
    assert_equal '<br this="tag" should="work">', Berns.void('br', this: 'tag', should: 'work')
  end

  def test_void_element_with_nested_attributes
    assert_equal '<br this="tag" should="work" data-foo="bar" data-bar-baz="foo">', Berns.void('br', this: 'tag', should: 'work', data: { foo: 'bar', bar: { baz: 'foo' } })
  end

  def test_simple_attributes
    assert_equal 'this="tag"', Berns.to_attributes(this: 'tag')
  end

  def test_deeply_nested_attributes
    assert_equal 'data-foo="bar" data-bar-baz="foo" data-bar-zoo-keeper="Bob"', Berns.to_attributes(data: { foo: 'bar', bar: { baz: 'foo', zoo: { keeper: 'Bob' } } })
  end

  def test_nested_attributes
    assert_equal 'this="tag" should="work" data-foo="bar" data-bar-baz="foo"', Berns.to_attributes(this: 'tag', should: 'work', data: { foo: 'bar', bar: { baz: 'foo' } })
    assert_equal %(data-something data-something-another="Foo"), Berns.to_attributes(data: { something: { nil => true, another: 'Foo' } })
    assert_equal %(href-stuff-another="foobar" href-blerg="Flerr"), Berns.to_attributes(href: { stuff: { another: 'foobar' }, blerg: 'Flerr' })
  end

  def test_empty_attributes
    assert_equal '', Berns.to_attributes({})
  end

  def test_escaped_attributes
    assert_equal 'this="&lt;&quot;tag&quot;"', Berns.to_attributes(this: '<"tag"')
  end

  def test_html_escape
    assert_equal '&lt;&quot;tag&quot;', Berns.escape_html('<"tag"')
  end

  def test_html_escape_invalid_types
    assert_raises(TypeError) do
      Berns.escape_html(:nope)
    end

    assert_raises(TypeError) do
      Berns.escape_html(['nope'])
    end

    assert_raises(TypeError) do
      Berns.escape_html({ no: 'pe' })
    end
  end

  def test_element
    assert_equal '<div></div>', Berns.element('div')
  end

  def test_element_from_symbol
    assert_equal '<div></div>', Berns.element(:div)
  end

  def test_element_with_block
    assert_equal '<div>Content</div>', Berns.element('div') { 'Content' }
  end

  def test_element_with_nil_block
    assert_equal '<div></div>', Berns.element('div') { nil }
  end

  def test_string_encoding
    assert_equal Encoding::UTF_8, Berns.element('div').encoding
    assert_equal Encoding::UTF_8, Berns.void('br').encoding
    assert_equal Encoding::UTF_8, Berns.to_attributes(href: { stuff: { another: 'foobar' }, blerg: 'Flerr' }).encoding
  end

  def test_void_element_methods
    %i[area base br col embed hr img input link menuitem meta param source track wbr].each do |void|
      assert_equal "<#{ void }>", Berns.send(void)
      assert_equal %(<#{ void } class="my-area-element">), Berns.send(void, class: 'my-area-element')
      assert_equal "<#{ void }>", Berns.send(void) { 'Content' }
    end
  end

  def test_standard_element_methods
    %i[a abbr address article aside audio b bdi bdo blockquote body button canvas caption cite code colgroup datalist dd del details dfn dialog div dl dt em fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i iframe ins kbd label legend li main map mark menu meter nav noscript object ol optgroup option output p picture pre progress q rp rt ruby s samp script section select small span strong style sub summary table tbody td template textarea tfoot th thead time title tr u ul var video].each do |standard|
      assert_equal "<#{ standard }></#{ standard }>", Berns.send(standard)
      assert_equal "<#{ standard }>Something</#{ standard }>", (Berns.send(standard) { 'Something' })
      assert_equal %(<#{ standard } class="something">Something</#{ standard }>), (Berns.send(standard, class: 'something') { 'Something' })
    end
  end

  def test_sanitize
    assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean</span>')
    assert_nil Berns.sanitize(nil)
  end

  def test_stupidity_of_element
    assert_equal '<br>ORLY</br>', Berns.element(:br) { 'ORLY' }
  end

  def test_to_attribute_boolean_values
    assert_equal 'nerf', Berns.to_attribute('nerf', true)
    assert_equal '', Berns.to_attribute('nerf', false)
  end

  def test_to_attribute
    assert_equal %(nerf="guns"), Berns.to_attribute('nerf', 'guns')
    assert_equal %(nerf-toy="guns"), Berns.to_attribute('nerf', 'toy' => 'guns')
  end

  def test_html_quote_escaping
    assert_equal %(foo="&lt;b&gt;bar&lt;/b&gt;-&quot;with &#39;quotes&#39;&quot;"), Berns.to_attribute('foo', %(<b>bar</b>-"with 'quotes'"))
  end

  def test_to_attribute_non_string_values
    assert_equal %(foo="4"), Berns.to_attribute('foo', 4)
    assert_equal %(foo="bar"), Berns.to_attribute('foo', :bar)
    assert_equal %(foo="[&quot;bar&quot;]"), Berns.to_attribute('foo', ['bar'])
  end
end
