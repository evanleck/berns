# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe 'Berns#sanitize' do
  it 'raises an error on anything but a string or nil' do
    assert_raises(TypeError) { Berns.sanitize(1) }
    assert_raises(TypeError) { Berns.sanitize(1.0) }
    assert_raises(TypeError) { Berns.sanitize(:symbol) }
    assert_raises(TypeError) { Berns.sanitize([]) }
    assert_raises(TypeError) { Berns.sanitize(Berns) }
    assert_raises(TypeError) { Berns.sanitize({}) }
  end

  it 'removes HTML from strings' do
    assert_equal 'This should be clean', Berns.sanitize('This should be clean')
    assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean</span>')
    assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean')
    assert_equal 'This should be clean', Berns.sanitize('This <span class="something">should be clean')
    assert_equal 'This should be clean', Berns.sanitize('This <span class="something">should be clean<')
    assert_equal 'This should be clean', Berns.sanitize('<br>This <span>should be <br><br><br />clean')
    assert_equal 'This should be clean', Berns.sanitize('&lt;This <span>should&gt; be <br><br><br />clean')
    assert_nil Berns.sanitize(nil)
  end

  it 'supports incomplete HTML' do
    assert_equal 'This ', Berns.sanitize('This <span never closes')
    assert_equal 'This ', Berns.sanitize('This &entity never closes')
  end
end
