# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe 'Berns#build' do
  it 'is a wrapper around Builder.new' do
    dom = Berns.build(title: 'TITLE!', para: 'PARAGRAPH!') do |title:, para:|
      h1 { title }
      p { para }
    end

    assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom
  end
end
