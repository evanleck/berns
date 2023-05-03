# frozen_string_literal: true
require 'berns'
require 'minitest/autorun'

describe Berns do
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
end
