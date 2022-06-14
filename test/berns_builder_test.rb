# frozen_string_literal: true
require 'berns/builder'
require 'minitest/autorun'

describe Berns::Builder do
  describe '#new' do
    it 'raises an argument error if a block is not passed' do
      assert_raises(ArgumentError) { Berns::Builder.new }
    end
  end

  describe '#element' do
    it 'allows creating plain elements' do
      dom = Berns::Builder.new do
        element(:div) { 'Content!' }
      end

      assert_equal '<div>Content!</div>', dom.call
      assert_equal '<div></div>', Berns::Builder.new { element('div') }.call
    end
  end

  describe '#void' do
    it 'allows creating void elements' do
      dom = Berns::Builder.new do
        void(:br, class: 'break')
        void(:hr)
      end

      assert_equal '<br class="break"><hr>', dom.call
      assert_equal '<br>', Berns::Builder.new { void('br') }.call
    end
  end

  describe '#text' do
    it 'appends text to the internal buffer and escapes the content' do
      dom = Berns::Builder.new do
        text '<h1>And & this'
      end

      assert_equal '&lt;h1&gt;And &amp; this', dom.call
    end
  end

  describe '#raw' do
    it 'appends text to the internal buffer without escaping the content' do
      dom = Berns::Builder.new do
        raw '<h1>And & this'
      end

      assert_equal '<h1>And & this', dom.call
    end
  end

  describe '#call' do
    it 'renders the template to a string' do
      dom = Berns::Builder.new { b { 'Bold!' } }
      assert_equal '<b>Bold!</b>', dom.call
    end

    it 'allows passing keyword arguments and raises an error when omitted' do
      dom = Berns::Builder.new do |title:, para:|
        h1 { title }
        p { para }
      end

      assert_raises(ArgumentError) { dom.call }
      assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom.call(title: 'TITLE!', para: 'PARAGRAPH!')
    end

    it 'allows passing positional arguments' do
      dom = Berns::Builder.new do |pos, pos2|
        h1 { pos }
        p { pos2 }
      end

      # Even though it seems like this _should_ raise an error, it doesn't
      # because Proc objects don't raise an ArgumentError for missing arguments
      # and instead fill in nils. Were this a lambda, it would raise an error.
      assert_equal %(<h1></h1><p></p>), dom.call
      assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom.call('TITLE!', 'PARAGRAPH!')
    end

    it 'allows passing both positional and keyword arguments and raises an error when omitted' do
      dom = Berns::Builder.new do |pos, title:|
        h1 { title }
        p { pos }
      end

      assert_raises(ArgumentError) { dom.call }
      assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom.call('PARAGRAPH!', title: 'TITLE!')
    end
  end

  describe '#to_s' do
    it 'renders the template to a string' do
      dom = Berns::Builder.new { b { 'Bold!' } }
      assert_equal '<b>Bold!</b>', dom.to_s
    end
  end

  describe '#to_str' do
    it 'renders the template to a string' do
      dom = Berns::Builder.new { b { 'Bold!' } }
      assert_equal '<b>Bold!</b>', dom.to_str
    end
  end

  describe 'standard elements' do
    Berns::STANDARD.each do |elm|
      it "appends an element using ##{ elm }" do
        assert_equal "<#{ elm }>Content</#{ elm }>", Berns::Builder.new { send(elm) { 'Content' } }.call
      end
    end
  end

  describe 'void elements' do
    Berns::VOID.each do |elm|
      it "appends an element using ##{ elm }" do
        assert_equal "<#{ elm }>", Berns::Builder.new { send(elm) }.call
      end
    end
  end

  describe 'block return values' do
    it 'does not capture the block return when the buffer has been modified' do
      dom = Berns::Builder.new do
        span { "This text is #{ b { 'bold' } }!" }
      end

      assert_equal '<span><b>bold</b></span>', dom.call
    end

    it 'does capture the block return when the buffer has not been modified' do
      dom = Berns::Builder.new do
        span { 'This text is plain.' }
      end

      assert_equal '<span>This text is plain.</span>', dom.call
    end

    it 'escapes the block return value when captured' do
      dom = Berns::Builder.new do
        span { 'This <b>text</b> is plain.' }
      end

      assert_equal '<span>This &lt;b&gt;text&lt;/b&gt; is plain.</span>', dom.call
    end
  end

  specify 'nesting' do
    dom = Berns::Builder.new do
      p(class: 'para') do
        text 'bare text'
        span(class: 'inline') { 'More text!' }
      end
    end

    assert_equal %(<p class="para">bare text<span class="inline">More text!</span></p>), dom.call
  end

  specify "Builder instances can be #call'ed multiple times" do
    dom = Berns::Builder.new do |name|
      h1 { name }
    end

    assert_equal '<h1>Bob</h1>', dom.call('Bob')
    assert_equal '<h1>Sue</h1>', dom.call('Sue')
  end

  specify "Builder instances return a frozen string when #call'ed" do
    dom = Berns::Builder.new do |name|
      h1 { name }
    end

    assert_predicate dom.call('Bob'), :frozen?
    assert_predicate dom.call('Sue'), :frozen?
  end
end
