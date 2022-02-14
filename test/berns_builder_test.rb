# frozen_string_literal: true
require 'berns/builder'
require 'minitest/autorun'

describe Berns::Builder do
  it "builds a DOM using Berns' own methods" do
    dom = Berns::Builder.new do
      h1 { 'Header' }
      text "where'd this come from?!"
      div
      p(class: 'para') { 'A paragraph!' }
    end

    assert_equal %(<h1>Header</h1>where&#39;d this come from?!<div></div><p class="para">A paragraph!</p>), dom.call
  end

  it 'allows nesting' do
    dom = Berns::Builder.new do
      p(class: 'para') do
        text 'bare text'
        span(class: 'inline') { 'More text!' }
      end
    end

    assert_equal %(<p class="para">bare text<span class="inline">More text!</span></p>), dom.call
  end

  it 'allows passing local variables and raises an error when omitted' do
    dom = Berns::Builder.new do |title:, para:|
      h1 { title }
      p { para }
    end

    assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom.call(title: 'TITLE!', para: 'PARAGRAPH!')

    assert_raises(ArgumentError) do
      dom.call
    end
  end

  it 'has access to the scope around where it was initialized' do
    opts = %w[one two three]

    dom = Berns::Builder.new do
      opts.each do |opt|
        span { opt }
      end
    end

    assert_equal %(<span>one</span><span>two</span><span>three</span>), dom.call
  end

  it 'can be initialized without a block and added to' do
    builder = Berns::Builder.new
    builder.h1 { 'Header' }

    assert_equal '<h1>Header</h1>', builder.to_s
  end

  it 'allows interpolation nesting in strings' do
    dom = Berns::Builder.new do
      span { "This text is #{ b { 'bold' } }!" }
    end

    assert_equal '<span>This text is <b>bold</b>!</span>', dom.call
  end

  it 'escapes text content' do
    dom = Berns::Builder.new do
      text '<h1>And & this'
    end

    assert_equal '&lt;h1&gt;And &amp; this', dom.call
  end
end
