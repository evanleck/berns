# frozen_string_literal: true
require_relative '../lib/berns'
require 'minitest/autorun'

describe Berns do
  def html
    Berns
  end

  describe '#sanitize' do
    it 'removes HTML markup from strings' do
      assert_equal 'This should be clean', html.sanitize('This <span>should be clean</span>')
    end

    it 'handles nil gracefully' do
      assert_nil html.sanitize(nil)
    end
  end

  describe '#element' do
    it 'generates HTML elements with text from a block' do
      assert_equal %(<a href="#nerds">Nerds!</a>), html.element(:a, href: '#nerds') { 'Nerds!' }
    end

    it 'is too dumb to know which elements should or should not contain text' do
      assert_equal '<br>ORLY</br>', html.element(:br) { 'ORLY' }
    end
  end

  describe '#void' do
    it 'generates HTML elements without text' do
      assert_equal '<br>', html.void(:br)
    end

    it 'is too dumb to know which elements should or should not be void' do
      assert_equal %(<a href="#nerds">), html.void(:a, href: '#nerds')
    end
  end

  describe '#to_attributes' do
    it 'converts a hash into a string of HTML attribute/value pairs' do
      assert_equal %(data-something data-something-another="Foo"), html.to_attributes(data: { something: { nil => true, another: 'Foo' } })
      assert_equal %(href-stuff-another="foobar" href-blerg="Flerr"), html.to_attributes(href: { stuff: { another: 'foobar' }, blerg: 'Flerr' })
    end
  end

  describe '#to_attribute' do
    it 'returns the attribute when passed true as the value' do
      assert_equal 'nerf', html.to_attribute('nerf', true)
    end

    it 'returns an empty string when passed false as the value' do
      assert_equal '', html.to_attribute('nerf', false)
    end

    it 'returns attribute/value pairs' do
      assert_equal %(nerf="guns"), html.to_attribute('nerf', 'guns')
    end

    it 'returns nested attribute/value pairs' do
      assert_equal %(nerf-toy="guns"), html.to_attribute('nerf', 'toy' => 'guns')
    end

    it 'escapes HTML and quotes' do
      assert_equal %(foo="&lt;b&gt;bar&lt;/b&gt;-&quot;with &#39;quotes&#39;&quot;"), html.to_attribute('foo', %(<b>bar</b>-"with 'quotes'"))
    end

    it 'works with non-string values' do
      assert_equal %(foo="4"), html.to_attribute('foo', 4)
      assert_equal %(foo="bar"), html.to_attribute('foo', :bar)
      assert_equal %(foo="[&quot;bar&quot;]"), html.to_attribute('foo', ['bar'])
    end
  end

  describe 'STANDARD' do
    Berns::STANDARD.each do |elm|
      it "generates #{ elm } standard elements from the html shortcut method" do
        assert_equal "<#{ elm }>✈️</#{ elm }>", html.send(elm) { '✈️' }
      end
    end
  end

  describe 'VOID' do
    Berns::VOID.each do |elm|
      it "generates #{ elm } void elements from the html shortcut method" do
        assert_equal "<#{ elm }>", html.send(elm)
      end
    end
  end
end
