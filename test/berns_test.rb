# frozen_string_literal: true
require 'berns'
require 'bigdecimal'
require 'minitest/autorun'

describe Berns do
  specify 'it has a version' do
    refute_nil Berns::VERSION
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

    it 'does not rewrite underscores in attribute names' do
      assert_equal %(data-foo_bar="baz"), Berns.to_attribute('data', 'foo_bar' => 'baz')
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

      # This one makes sure *args and **kwargs is covered appropriately.
      assert_equal 'this="tag" should="work" data-foo="bar" data-bar-baz="foo"', Berns.to_attributes('this' => 'tag', should: 'work', data: { 'foo' => 'bar', bar: { baz: 'foo' } })
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

    it 'escapes attribute values' do
      assert_equal %(data="&lt;&quot;tag&quot;"), Berns.to_attributes(data: '<"tag"')
    end

    it 'handles large hashes' do
      huge = (0..256).each_with_object({}) do |count, attrs|
        attrs["data-#{ count }"] = "This is data attribute number #{ count }"
      end

      expected = %(data-0="This is data attribute number 0" data-1="This is data attribute number 1" data-2="This is data attribute number 2" data-3="This is data attribute number 3" data-4="This is data attribute number 4" data-5="This is data attribute number 5" data-6="This is data attribute number 6" data-7="This is data attribute number 7" data-8="This is data attribute number 8" data-9="This is data attribute number 9" data-10="This is data attribute number 10" data-11="This is data attribute number 11" data-12="This is data attribute number 12" data-13="This is data attribute number 13" data-14="This is data attribute number 14" data-15="This is data attribute number 15" data-16="This is data attribute number 16" data-17="This is data attribute number 17" data-18="This is data attribute number 18" data-19="This is data attribute number 19" data-20="This is data attribute number 20" data-21="This is data attribute number 21" data-22="This is data attribute number 22" data-23="This is data attribute number 23" data-24="This is data attribute number 24" data-25="This is data attribute number 25" data-26="This is data attribute number 26" data-27="This is data attribute number 27" data-28="This is data attribute number 28" data-29="This is data attribute number 29" data-30="This is data attribute number 30" data-31="This is data attribute number 31" data-32="This is data attribute number 32" data-33="This is data attribute number 33" data-34="This is data attribute number 34" data-35="This is data attribute number 35" data-36="This is data attribute number 36" data-37="This is data attribute number 37" data-38="This is data attribute number 38" data-39="This is data attribute number 39" data-40="This is data attribute number 40" data-41="This is data attribute number 41" data-42="This is data attribute number 42" data-43="This is data attribute number 43" data-44="This is data attribute number 44" data-45="This is data attribute number 45" data-46="This is data attribute number 46" data-47="This is data attribute number 47" data-48="This is data attribute number 48" data-49="This is data attribute number 49" data-50="This is data attribute number 50" data-51="This is data attribute number 51" data-52="This is data attribute number 52" data-53="This is data attribute number 53" data-54="This is data attribute number 54" data-55="This is data attribute number 55" data-56="This is data attribute number 56" data-57="This is data attribute number 57" data-58="This is data attribute number 58" data-59="This is data attribute number 59" data-60="This is data attribute number 60" data-61="This is data attribute number 61" data-62="This is data attribute number 62" data-63="This is data attribute number 63" data-64="This is data attribute number 64" data-65="This is data attribute number 65" data-66="This is data attribute number 66" data-67="This is data attribute number 67" data-68="This is data attribute number 68" data-69="This is data attribute number 69" data-70="This is data attribute number 70" data-71="This is data attribute number 71" data-72="This is data attribute number 72" data-73="This is data attribute number 73" data-74="This is data attribute number 74" data-75="This is data attribute number 75" data-76="This is data attribute number 76" data-77="This is data attribute number 77" data-78="This is data attribute number 78" data-79="This is data attribute number 79" data-80="This is data attribute number 80" data-81="This is data attribute number 81" data-82="This is data attribute number 82" data-83="This is data attribute number 83" data-84="This is data attribute number 84" data-85="This is data attribute number 85" data-86="This is data attribute number 86" data-87="This is data attribute number 87" data-88="This is data attribute number 88" data-89="This is data attribute number 89" data-90="This is data attribute number 90" data-91="This is data attribute number 91" data-92="This is data attribute number 92" data-93="This is data attribute number 93" data-94="This is data attribute number 94" data-95="This is data attribute number 95" data-96="This is data attribute number 96" data-97="This is data attribute number 97" data-98="This is data attribute number 98" data-99="This is data attribute number 99" data-100="This is data attribute number 100" data-101="This is data attribute number 101" data-102="This is data attribute number 102" data-103="This is data attribute number 103" data-104="This is data attribute number 104" data-105="This is data attribute number 105" data-106="This is data attribute number 106" data-107="This is data attribute number 107" data-108="This is data attribute number 108" data-109="This is data attribute number 109" data-110="This is data attribute number 110" data-111="This is data attribute number 111" data-112="This is data attribute number 112" data-113="This is data attribute number 113" data-114="This is data attribute number 114" data-115="This is data attribute number 115" data-116="This is data attribute number 116" data-117="This is data attribute number 117" data-118="This is data attribute number 118" data-119="This is data attribute number 119" data-120="This is data attribute number 120" data-121="This is data attribute number 121" data-122="This is data attribute number 122" data-123="This is data attribute number 123" data-124="This is data attribute number 124" data-125="This is data attribute number 125" data-126="This is data attribute number 126" data-127="This is data attribute number 127" data-128="This is data attribute number 128" data-129="This is data attribute number 129" data-130="This is data attribute number 130" data-131="This is data attribute number 131" data-132="This is data attribute number 132" data-133="This is data attribute number 133" data-134="This is data attribute number 134" data-135="This is data attribute number 135" data-136="This is data attribute number 136" data-137="This is data attribute number 137" data-138="This is data attribute number 138" data-139="This is data attribute number 139" data-140="This is data attribute number 140" data-141="This is data attribute number 141" data-142="This is data attribute number 142" data-143="This is data attribute number 143" data-144="This is data attribute number 144" data-145="This is data attribute number 145" data-146="This is data attribute number 146" data-147="This is data attribute number 147" data-148="This is data attribute number 148" data-149="This is data attribute number 149" data-150="This is data attribute number 150" data-151="This is data attribute number 151" data-152="This is data attribute number 152" data-153="This is data attribute number 153" data-154="This is data attribute number 154" data-155="This is data attribute number 155" data-156="This is data attribute number 156" data-157="This is data attribute number 157" data-158="This is data attribute number 158" data-159="This is data attribute number 159" data-160="This is data attribute number 160" data-161="This is data attribute number 161" data-162="This is data attribute number 162" data-163="This is data attribute number 163" data-164="This is data attribute number 164" data-165="This is data attribute number 165" data-166="This is data attribute number 166" data-167="This is data attribute number 167" data-168="This is data attribute number 168" data-169="This is data attribute number 169" data-170="This is data attribute number 170" data-171="This is data attribute number 171" data-172="This is data attribute number 172" data-173="This is data attribute number 173" data-174="This is data attribute number 174" data-175="This is data attribute number 175" data-176="This is data attribute number 176" data-177="This is data attribute number 177" data-178="This is data attribute number 178" data-179="This is data attribute number 179" data-180="This is data attribute number 180" data-181="This is data attribute number 181" data-182="This is data attribute number 182" data-183="This is data attribute number 183" data-184="This is data attribute number 184" data-185="This is data attribute number 185" data-186="This is data attribute number 186" data-187="This is data attribute number 187" data-188="This is data attribute number 188" data-189="This is data attribute number 189" data-190="This is data attribute number 190" data-191="This is data attribute number 191" data-192="This is data attribute number 192" data-193="This is data attribute number 193" data-194="This is data attribute number 194" data-195="This is data attribute number 195" data-196="This is data attribute number 196" data-197="This is data attribute number 197" data-198="This is data attribute number 198" data-199="This is data attribute number 199" data-200="This is data attribute number 200" data-201="This is data attribute number 201" data-202="This is data attribute number 202" data-203="This is data attribute number 203" data-204="This is data attribute number 204" data-205="This is data attribute number 205" data-206="This is data attribute number 206" data-207="This is data attribute number 207" data-208="This is data attribute number 208" data-209="This is data attribute number 209" data-210="This is data attribute number 210" data-211="This is data attribute number 211" data-212="This is data attribute number 212" data-213="This is data attribute number 213" data-214="This is data attribute number 214" data-215="This is data attribute number 215" data-216="This is data attribute number 216" data-217="This is data attribute number 217" data-218="This is data attribute number 218" data-219="This is data attribute number 219" data-220="This is data attribute number 220" data-221="This is data attribute number 221" data-222="This is data attribute number 222" data-223="This is data attribute number 223" data-224="This is data attribute number 224" data-225="This is data attribute number 225" data-226="This is data attribute number 226" data-227="This is data attribute number 227" data-228="This is data attribute number 228" data-229="This is data attribute number 229" data-230="This is data attribute number 230" data-231="This is data attribute number 231" data-232="This is data attribute number 232" data-233="This is data attribute number 233" data-234="This is data attribute number 234" data-235="This is data attribute number 235" data-236="This is data attribute number 236" data-237="This is data attribute number 237" data-238="This is data attribute number 238" data-239="This is data attribute number 239" data-240="This is data attribute number 240" data-241="This is data attribute number 241" data-242="This is data attribute number 242" data-243="This is data attribute number 243" data-244="This is data attribute number 244" data-245="This is data attribute number 245" data-246="This is data attribute number 246" data-247="This is data attribute number 247" data-248="This is data attribute number 248" data-249="This is data attribute number 249" data-250="This is data attribute number 250" data-251="This is data attribute number 251" data-252="This is data attribute number 252" data-253="This is data attribute number 253" data-254="This is data attribute number 254" data-255="This is data attribute number 255" data-256="This is data attribute number 256")

      assert_equal expected, Berns.to_attributes(huge)
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

  describe '#element' do
    it 'is does not check the element type' do
      assert_equal '<br>ORLY</br>', Berns.element(:br) { 'ORLY' }
    end

    it 'creates empty standard elements' do
      assert_equal '<div></div>', Berns.element('div')
      assert_equal '<div></div>', Berns.element(:div)
    end

    it 'creates empty standard elements with attributes' do
      assert_equal '<div></div>', Berns.element('div', {})
      assert_equal '<div></div>', Berns.element(:div, {})
      assert_equal '<div one="One"></div>', Berns.element('div', { one: 'One' })
      assert_equal '<div one="One" two="Two"></div>', Berns.element('div', { one: 'One', two: 'Two' })
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
      assert_equal 'This should be clean', Berns.sanitize('This should be clean')
      assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean</span>')
      assert_equal 'This should be clean', Berns.sanitize('This <span>should be clean')
      assert_equal 'This should be clean', Berns.sanitize('This <span class="something">should be clean')
      assert_equal 'This should be clean', Berns.sanitize('This <span class="something">should be clean<')
      assert_equal 'This should be clean', Berns.sanitize('<br>This <span>should be <br><br><br />clean')
      assert_equal 'This should be clean', Berns.sanitize('&lt;This <span>should&gt; be <br><br><br />clean')
      assert_nil Berns.sanitize(nil)
    end
  end

  describe '#build' do
    it 'is a wrapper around Builder.new' do
      dom = Berns.build(title: 'TITLE!', para: 'PARAGRAPH!') do |title:, para:|
        h1 { title }
        p { para }
      end

      assert_equal %(<h1>TITLE!</h1><p>PARAGRAPH!</p>), dom
    end
  end
end
