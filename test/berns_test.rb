# frozen_string_literal: true
require "test_helper"

describe Berns do
  def html
    Berns
  end

  describe '#element' do
    it 'generates HTML elements with text from a block' do
      assert_equal html.element(:a, href: '#nerds') { 'Nerds!' }, "<a href='#nerds'>Nerds!</a>"
    end

    it 'is too dumb to know which elements should or should not contain text' do
      assert_equal html.element(:br) { 'ORLY' }, '<br>ORLY</br>'
    end
  end

  describe '#to_attributes' do
    it 'converts a hash into a string of HTML attribute/value pairs' do
      assert_equal html.to_attributes(href: { stuff: { another: 'foobar' }, blerg: 'Flerr' }), "href-stuff-another='foobar' href-blerg='Flerr'"
    end
  end

  describe '#to_attribute' do
    it 'returns the attribute when passed true as the value' do
      assert_equal html.to_attribute('nerf', true), 'nerf'
    end

    it 'returns an empty string when passed false as the value' do
      assert_equal html.to_attribute('nerf', false), ''
    end

    it 'returns attribute/value pairs' do
      assert_equal "nerf='guns'", html.to_attribute('nerf', 'guns')
    end

    it 'returns nested attribute/value pairs' do
      assert_equal "nerf-toy='guns'", html.to_attribute('nerf', 'toy' => 'guns')
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
        assert_equal "<#{elm}>", html.send(elm)
      end
    end
  end
end
