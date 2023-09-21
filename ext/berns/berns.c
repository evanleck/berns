#include <stdbool.h>

#include "hescape.h"
#include "ruby.h"
#include "strxcat.h"
#include "strxcpy.h"
#include "strxempty.h"
#include "strxfree.h"
#include "strxnew.h"
#include "strxresize.h"

static const char *attr_close = "\"";
static const size_t attr_clen = 1;

static const char *attr_equals = "=\"";
static const size_t attr_eqlen = 2;

static const char *dash = "-";
static const size_t dlen = 1;

static const char *space = " ";
static const size_t splen = 1;

static const char *tag_open = "<";
static const size_t tag_olen = 1;

static const char *tag_close = ">";
static const size_t tag_clen = 1;

static const char *slash = "/";
static const size_t sllen = 1;


/*
 * Macro to capture a block's content as a Ruby string into the local variable
 * content.
 */
#define CONTENT_FROM_BLOCK \
	VALUE content; \
	\
	if (rb_block_given_p()) { \
		content = rb_yield(Qnil); \
	\
		if (TYPE(content) == T_NIL || TYPE(content) == T_FALSE) { \
			content = rb_utf8_str_new_cstr(""); \
		} else if (TYPE(content) != T_STRING) { \
			content = rb_funcall(content, rb_intern("to_s"), 0); \
		} \
	} else { \
		content = rb_utf8_str_new_cstr(""); \
	}

/*
 * Macro to define a "dynamic" function that generates a void element.
 */
#define VOID_ELEMENT(element_name) \
	static VALUE external_##element_name##_element(int argc, VALUE *argv, RB_UNUSED_VAR(VALUE self)) { \
		rb_check_arity(argc, 0, 1); \
		\
		const char *tag = #element_name; \
		char *string; \
		\
		if (argc == 1) { \
			string = void_element_with_attributes(tag, strlen(tag), argv[0]); \
		} else { \
			string = void_element_without_attributes(tag, strlen(tag)); \
		} \
		VALUE rstring = rb_utf8_str_new_cstr(string); \
		strxfree(string); \
		\
		return rstring; \
	}

/*
 * Macro to define a "dynamic" function that generates a standard element.
 */
#define STANDARD_ELEMENT(element_name) \
	static VALUE external_##element_name##_element(int argc, VALUE *argv, RB_UNUSED_VAR(VALUE self)) { \
		rb_check_arity(argc, 0, 1); \
		\
		CONTENT_FROM_BLOCK \
		const char *tag = #element_name; \
		char *string; \
		\
		if (argc == 1) { \
			string = element_with_attributes(tag, strlen(tag), RSTRING_PTR(content), RSTRING_LEN(content), argv[0]); \
		} else { \
			string = element_without_attributes(tag, strlen(tag), RSTRING_PTR(content), RSTRING_LEN(content)); \
		} \
		VALUE rstring = rb_utf8_str_new_cstr(string); \
		strxfree(string); \
		\
		return rstring; \
	}

/*
 * The external API for Berns.sanitize
 *
 * string should be a string or nil, anything else will raise an error.
 *
 */
static VALUE external_sanitize(RB_UNUSED_VAR(VALUE self), VALUE string) {
	if (TYPE(string) == T_NIL) {
		return Qnil;
	}

	Check_Type(string, T_STRING);

	size_t slen = RSTRING_LEN(string);
	char *str = RSTRING_PTR(string);

	char dest[slen + 1];

	bool entity = false;
	bool modified = false;
	bool open = false;
	unsigned int index = 0;

	for (unsigned int i = 0; i < slen; i++) {
		switch(str[i]) {
			case '<':
				open = true;
				modified = true;
				break;
			case '>':
				open = false;
				break;
			case '&':
				entity = true;
				modified = true;
				break;
			case ';':
				entity = false;
				break;
			default:
				if (!open && !entity) {
					dest[index++] = str[i];
				}

				break;
		}
	}

	dest[index] = '\0';

	/*
	 * If the string was never modified, return the original string, otherwise
	 * create a new string from our destination buffer.
	 */
	if (modified) {
		return rb_utf8_str_new_cstr(dest);
	} else {
		return string;
	}
}

/*
 * The external API for Berns.escape_html.
 *
 * Anything other than a string will raise an error.
 *
 */
static VALUE external_escape_html(RB_UNUSED_VAR(VALUE self), VALUE string) {
	Check_Type(string, T_STRING);

	uint8_t *dest = NULL;
	size_t slen = RSTRING_LEN(string);
	size_t esclen = hesc_escape_html(&dest, RSTRING_PTR(string), slen);

	VALUE rstring;

	if (esclen > slen) {
		rstring = rb_utf8_str_new_cstr(dest);
		free(dest);
	} else {
		rstring = string;
	}

	return rstring;
}

/*
 * Return a freeable piece of memory with a copy of the attribute passed in it.
 * Why does this exist? So we can free the memory created by this without having
 * to branch further in other places.
 */
static inline char * empty_value_to_attribute(const char *attr, const size_t attrlen) {
	size_t total_size = attrlen + 1;
	char *dest = strxnew(total_size);

	strxcat(dest, total_size, attr);

	return dest;
}

/*
 * Takes a string attribute name and value pair and converts them into a string
 * ready to use in HTML
 */
static char * string_value_to_attribute(const char *attr, const size_t attrlen, const char *value, const size_t vallen) {
	if (vallen == 0) {
		size_t total_size = attrlen + 1;
		char *dest = strxnew(total_size);

		strxcat(dest, total_size, attr);

		return dest;
	} else {
		uint8_t *edest = NULL;
		size_t esclen = hesc_escape_html(&edest, value, vallen);

		size_t total_size = attrlen + attr_eqlen + esclen + attr_clen + 1;
		char *dest = strxnew(total_size);

		strxcat(dest, total_size, attr, attr_equals, edest, attr_close);

		if (esclen > vallen) {
			free(edest);
		}

		return dest;
	}
}

static char * hash_value_to_attribute(const char *attr, const size_t attrlen, VALUE value) {
	Check_Type(value, T_HASH);

	if (RHASH_SIZE(value) == 0) {
		return strxempty();
	}

	VALUE subkey;
	VALUE subvalue;

	const VALUE keys = rb_funcall(value, rb_intern("keys"), 0);
	const VALUE length = RARRAY_LEN(keys);

	size_t allocated = 256;
	size_t occupied = 0;

	char *destination = strxnew(allocated);
	char *position = destination;

	for (unsigned int i = 0; i < length; i++) {
		subkey = rb_ary_entry(keys, i);
		subvalue = rb_hash_aref(value, subkey);

		switch(TYPE(subkey)) {
			case T_STRING:
				break;
			case T_NIL:
				subkey = rb_utf8_str_new_cstr("");
				break;
			case T_SYMBOL:
				subkey = rb_sym2str(subkey);
				break;
			default:
				strxfree(destination);
				rb_raise(rb_eTypeError, "Berns.to_attribute value keys must be Strings, Symbols, or nil.");
				break;
		}

		size_t subattr_len = attrlen;
		size_t subkey_len = RSTRING_LEN(subkey);

		if (attrlen > 0 && subkey_len > 0) {
			subattr_len += dlen;
		}

		if (subkey_len > 0) {
			subattr_len += subkey_len;
		}

		char subattr[subattr_len + 1];
		char *ptr = subattr;
		char *subend = subattr + subattr_len + 1;

		if (attrlen > 0) {
			ptr = strxcat(ptr, attrlen + 1, attr);
		}

		if (attrlen > 0 && subkey_len > 0) {
			ptr = strxcat(ptr, dlen + 1, dash);
		}

		strxcat(ptr, subkey_len + 1, RSTRING_PTR(subkey));

		char *combined;

		switch(TYPE(subvalue)) {
			case T_FALSE:
				combined = strxempty();
				break;

			case T_NIL:
				/* Fall through. */
			case T_TRUE:
				combined = empty_value_to_attribute(subattr, subattr_len);
				break;

			case T_STRING:
				combined = string_value_to_attribute(subattr, subattr_len, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
				break;

			case T_SYMBOL:
				subvalue = rb_sym2str(subvalue);
				combined = string_value_to_attribute(subattr, subattr_len, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
				break;

			case T_HASH:
				combined = hash_value_to_attribute(subattr, subattr_len, subvalue);
				break;

			default:
				subvalue = rb_funcall(subvalue, rb_intern("to_s"), 0);
				combined = string_value_to_attribute(subattr, subattr_len, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
				break;
		}

		size_t combined_len = strlen(combined);
		size_t size_to_append = combined_len + 1;

		if (i > 0) {
			size_to_append += splen;
		}

		if ((size_to_append + occupied) > allocated) {
			/* To avoid an abundance of reallocations, this is a multiple of 256. */
			double multiple = (double) (size_to_append + occupied) / 256;
			size_t new_size_to_allocate = (unsigned int) ceil(multiple) * 256;

			char *tmp = strxresize(destination, new_size_to_allocate);

			allocated = new_size_to_allocate;
			destination = tmp;
			position = destination + occupied;
		}

		if (i > 0) {
			position = strxcat(position, splen + 1, space);
			occupied += splen;
		}

		position = strxcat(position, combined_len + 1, combined);
		occupied += combined_len;

		strxfree(combined);
	}

	/*
	 * Reallocate destination to final size. This is generally a reduction in the
	 * allocated memory since we chunk allocations in 256 byte multiples.
	 */
	char *rightsizeddest = strxresize(destination, occupied + 1);

	rightsizeddest[occupied] = '\0';

	return rightsizeddest;
}

/*
 * Convert an attribute name and value into a string.
 */
static char * to_attribute(VALUE attr, VALUE value) {
	switch(TYPE(attr)) {
		case T_SYMBOL:
			attr = rb_sym2str(attr);
			break;
		default:
			break;
	}

	Check_Type(attr, T_STRING);

	char *val = NULL;
	VALUE str;

	switch(TYPE(value)) {
		case T_NIL:
			/* Fall through. */
		case T_TRUE:
			val = empty_value_to_attribute(RSTRING_PTR(attr), RSTRING_LEN(attr));
			break;
		case T_FALSE:
			val = strxempty();
			break;
		case T_HASH:
			val = hash_value_to_attribute(RSTRING_PTR(attr), RSTRING_LEN(attr), value);
			break;
		case T_STRING:
			val = string_value_to_attribute(RSTRING_PTR(attr), RSTRING_LEN(attr), RSTRING_PTR(value), RSTRING_LEN(value));
			break;
		case T_SYMBOL:
			str = rb_sym2str(value);
			val = string_value_to_attribute(RSTRING_PTR(attr), RSTRING_LEN(attr), RSTRING_PTR(str), RSTRING_LEN(str));
			break;
		default:
			str = rb_funcall(value, rb_intern("to_s"), 0);
			val = string_value_to_attribute(RSTRING_PTR(attr), RSTRING_LEN(attr), RSTRING_PTR(str), RSTRING_LEN(str));
			break;
	}

	return val;
}

/*
 * The external API for Berns.to_attribute.
 *
 * attr should be either a symbol or string, otherwise an error is raised. value
 * can be anything to responds to #to_s
 *
 */
static VALUE external_to_attribute(RB_UNUSED_VAR(VALUE self), VALUE attr, VALUE value) {
	if (TYPE(attr) == T_SYMBOL) {
		attr = rb_sym2str(attr);
	}

	Check_Type(attr, T_STRING);

	char *val = to_attribute(attr, value);
	VALUE rstring = rb_utf8_str_new_cstr(val);
	strxfree(val);

	return rstring;
}

/*
 * The external API for Berns.to_attributes.
 *
 * attributes should be a hash, otherwise an error is raised.
 *
 */
static VALUE external_to_attributes(RB_UNUSED_VAR(VALUE self), VALUE attributes) {
	Check_Type(attributes, T_HASH);

	if (RHASH_SIZE(attributes) == 0) {
		return rb_utf8_str_new_cstr("");
	}

	char *attrs = hash_value_to_attribute("", 0, attributes);
	VALUE rstring = rb_utf8_str_new_cstr(attrs);
	strxfree(attrs);

	return rstring;
}

/*
 * Create a void element i.e. one without children/content but with attributes.
 */
static char * void_element_with_attributes(const char *tag, size_t tlen, VALUE attributes) {
	char *attrs = hash_value_to_attribute("", 0, attributes);

	size_t alen = strlen(attrs);
	size_t total = tag_olen + tlen + tag_clen + 1;

	/* If we have some attributes, add a space and the attributes' length. */
	if (alen > 0) {
		total += splen + alen;
	}

	char *dest = strxnew(total);
	char *ptr = strxcat(dest, tag_olen + tlen + 1, tag_open, tag);

	if (alen > 0) {
		ptr = strxcat(ptr, splen + alen + 1, space, attrs);
	}

	strxcat(ptr, tag_clen + 1, tag_close);
	strxfree(attrs);

	return dest;
}

/*
 * Create a void element i.e. one without children/content or attributes.
 */
static char * void_element_without_attributes(const char *tag, size_t tlen) {
	size_t total = tag_olen + tlen + tag_clen + 1;

	char *dest = strxnew(total);
	strxcat(dest, total, tag_open, tag, tag_close);

	return dest;
}

/*
 * The external API for Berns.void.
 *
 * The first argument should be a string or symbol, otherwise an error is
 * raised. The second argument must be a hash if present.
 *
 */
static VALUE external_void_element(int argc, VALUE *arguments, RB_UNUSED_VAR(VALUE self)) {
	rb_check_arity(argc, 1, 2);

	VALUE tag = arguments[0];
	VALUE attributes = arguments[1];

	if (TYPE(tag) == T_SYMBOL) {
		tag = rb_sym2str(tag);
	}

	Check_Type(tag, T_STRING);

	char *string;

	if (argc == 2) {
		string = void_element_with_attributes(RSTRING_PTR(tag), RSTRING_LEN(tag), attributes);
	} else {
		string = void_element_without_attributes(RSTRING_PTR(tag), RSTRING_LEN(tag));
	}

	VALUE rstring = rb_utf8_str_new_cstr(string);
	strxfree(string);

	return rstring;
}

static char * element_with_attributes(const char *tag, size_t tlen, char *content, size_t conlen, VALUE attributes) {
	char *attrs = hash_value_to_attribute("", 0, attributes);
	size_t alen = strlen(attrs);

	size_t total = tag_olen + tlen + tag_clen + tag_olen + sllen + tlen + tag_clen + 1;

	/* If we have some attributes, add a space and the attributes' length. */
	if (alen > 0) {
		total += splen + alen;
	}

	/* If we have some content, add the content length to our total. */
	if (conlen > 0) {
		total += conlen;
	}

	char *dest = strxnew(total);
	char *ptr = strxcat(dest, tag_olen + tlen + 1, tag_open, tag);

	if (alen > 0) {
		ptr = strxcat(ptr, splen + alen + 1, space, attrs);
	}

	ptr = strxcat(ptr, tag_clen + 1, tag_close);

	if (conlen > 0) {
		ptr = strxcat(ptr, conlen + 1, content);
	}

	ptr = strxcat(ptr, tag_olen + sllen + tlen + tag_clen + 1, tag_open, slash, tag, tag_close);
	strxfree(attrs);

	return dest;
}

static char * element_without_attributes(const char *tag, size_t tlen, char *content, size_t conlen) {
	size_t total = tag_olen + tlen + tag_clen + tag_olen + sllen + tlen + tag_clen + 1;

	/* If we have some content, add the content length to our total. */
	if (conlen > 0) {
		total += conlen;
	}

	char *dest = strxnew(total);
	char *ptr = strxcat(dest, tag_olen + tlen + tag_clen + 1, tag_open, tag, tag_close);

	if (conlen > 0) {
		ptr = strxcat(ptr, conlen + 1, content);
	}

	strxcat(ptr, tag_olen + sllen + tlen + tag_clen + 1, tag_open, slash, tag, tag_close);

	return dest;
}

/*
 * The external API for Berns.element.
 *
 * The first argument should be a string or symbol, otherwise an error is
 * raised. The second argument must be a hash if present. An optional block can
 * be given which will used as the contents of the element.
 *
 */
static VALUE external_element(int argc, VALUE *arguments, RB_UNUSED_VAR(VALUE self)) {
	rb_check_arity(argc, 1, 2);

	VALUE tag = arguments[0];

	if (TYPE(tag) == T_SYMBOL) {
		tag = rb_sym2str(tag);
	}

	Check_Type(tag, T_STRING);

	CONTENT_FROM_BLOCK

	char *string;

	if (argc == 2) {
		string = element_with_attributes(RSTRING_PTR(tag), RSTRING_LEN(tag), RSTRING_PTR(content), RSTRING_LEN(content), arguments[1]);
	} else {
		string = element_without_attributes(RSTRING_PTR(tag), RSTRING_LEN(tag), RSTRING_PTR(content), RSTRING_LEN(content));
	}

	VALUE rstring = rb_utf8_str_new_cstr(string);
	strxfree(string);

	return rstring;
}

VOID_ELEMENT(area)
VOID_ELEMENT(base)
VOID_ELEMENT(br)
VOID_ELEMENT(col)
VOID_ELEMENT(embed)
VOID_ELEMENT(hr)
VOID_ELEMENT(img)
VOID_ELEMENT(input)
VOID_ELEMENT(link)
VOID_ELEMENT(menuitem)
VOID_ELEMENT(meta)
VOID_ELEMENT(param)
VOID_ELEMENT(source)
VOID_ELEMENT(track)
VOID_ELEMENT(wbr)

STANDARD_ELEMENT(a)
STANDARD_ELEMENT(abbr)
STANDARD_ELEMENT(address)
STANDARD_ELEMENT(article)
STANDARD_ELEMENT(aside)
STANDARD_ELEMENT(audio)
STANDARD_ELEMENT(b)
STANDARD_ELEMENT(bdi)
STANDARD_ELEMENT(bdo)
STANDARD_ELEMENT(blockquote)
STANDARD_ELEMENT(body)
STANDARD_ELEMENT(button)
STANDARD_ELEMENT(canvas)
STANDARD_ELEMENT(caption)
STANDARD_ELEMENT(cite)
STANDARD_ELEMENT(code)
STANDARD_ELEMENT(colgroup)
STANDARD_ELEMENT(datalist)
STANDARD_ELEMENT(dd)
STANDARD_ELEMENT(del)
STANDARD_ELEMENT(details)
STANDARD_ELEMENT(dfn)
STANDARD_ELEMENT(dialog)
STANDARD_ELEMENT(div)
STANDARD_ELEMENT(dl)
STANDARD_ELEMENT(dt)
STANDARD_ELEMENT(em)
STANDARD_ELEMENT(fieldset)
STANDARD_ELEMENT(figcaption)
STANDARD_ELEMENT(figure)
STANDARD_ELEMENT(footer)
STANDARD_ELEMENT(form)
STANDARD_ELEMENT(h1)
STANDARD_ELEMENT(h2)
STANDARD_ELEMENT(h3)
STANDARD_ELEMENT(h4)
STANDARD_ELEMENT(h5)
STANDARD_ELEMENT(h6)
STANDARD_ELEMENT(head)
STANDARD_ELEMENT(header)
STANDARD_ELEMENT(html)
STANDARD_ELEMENT(i)
STANDARD_ELEMENT(iframe)
STANDARD_ELEMENT(ins)
STANDARD_ELEMENT(kbd)
STANDARD_ELEMENT(label)
STANDARD_ELEMENT(legend)
STANDARD_ELEMENT(li)
STANDARD_ELEMENT(main)
STANDARD_ELEMENT(map)
STANDARD_ELEMENT(mark)
STANDARD_ELEMENT(menu)
STANDARD_ELEMENT(meter)
STANDARD_ELEMENT(nav)
STANDARD_ELEMENT(noscript)
STANDARD_ELEMENT(object)
STANDARD_ELEMENT(ol)
STANDARD_ELEMENT(optgroup)
STANDARD_ELEMENT(option)
STANDARD_ELEMENT(output)
STANDARD_ELEMENT(p)
STANDARD_ELEMENT(picture)
STANDARD_ELEMENT(pre)
STANDARD_ELEMENT(progress)
STANDARD_ELEMENT(q)
STANDARD_ELEMENT(rp)
STANDARD_ELEMENT(rt)
STANDARD_ELEMENT(ruby)
STANDARD_ELEMENT(s)
STANDARD_ELEMENT(samp)
STANDARD_ELEMENT(script)
STANDARD_ELEMENT(section)
STANDARD_ELEMENT(select)
STANDARD_ELEMENT(small)
STANDARD_ELEMENT(span)
STANDARD_ELEMENT(strong)
STANDARD_ELEMENT(style)
STANDARD_ELEMENT(sub)
STANDARD_ELEMENT(summary)
STANDARD_ELEMENT(table)
STANDARD_ELEMENT(tbody)
STANDARD_ELEMENT(td)
STANDARD_ELEMENT(template)
STANDARD_ELEMENT(textarea)
STANDARD_ELEMENT(tfoot)
STANDARD_ELEMENT(th)
STANDARD_ELEMENT(thead)
STANDARD_ELEMENT(time)
STANDARD_ELEMENT(title)
STANDARD_ELEMENT(tr)
STANDARD_ELEMENT(u)
STANDARD_ELEMENT(ul)
STANDARD_ELEMENT(var)
STANDARD_ELEMENT(video)

void Init_berns() {
	VALUE Berns = rb_define_module("Berns");

	rb_define_singleton_method(Berns, "element", external_element, -1);
	rb_define_singleton_method(Berns, "escape_html", external_escape_html, 1);
	rb_define_singleton_method(Berns, "sanitize", external_sanitize, 1);
	rb_define_singleton_method(Berns, "to_attribute", external_to_attribute, 2);
	rb_define_singleton_method(Berns, "to_attributes", external_to_attributes, 1);
	rb_define_singleton_method(Berns, "void", external_void_element, -1);

	/*
	 * List of void elements - http://xahlee.info/js/html5_non-closing_tag.html
	 *
	 * area base br col embed hr img input link menuitem meta param source track
	 * wbr
	 *
	 */
	rb_define_singleton_method(Berns, "area", external_area_element, -1);
	rb_define_singleton_method(Berns, "base", external_base_element, -1);
	rb_define_singleton_method(Berns, "br", external_br_element, -1);
	rb_define_singleton_method(Berns, "col", external_col_element, -1);
	rb_define_singleton_method(Berns, "embed", external_embed_element, -1);
	rb_define_singleton_method(Berns, "hr", external_hr_element, -1);
	rb_define_singleton_method(Berns, "img", external_img_element, -1);
	rb_define_singleton_method(Berns, "input", external_input_element, -1);
	rb_define_singleton_method(Berns, "link", external_link_element, -1);
	rb_define_singleton_method(Berns, "menuitem", external_menuitem_element, -1);
	rb_define_singleton_method(Berns, "meta", external_meta_element, -1);
	rb_define_singleton_method(Berns, "param", external_param_element, -1);
	rb_define_singleton_method(Berns, "source", external_source_element, -1);
	rb_define_singleton_method(Berns, "track", external_track_element, -1);
	rb_define_singleton_method(Berns, "wbr", external_wbr_element, -1);

	/*
	 * List of standard HTML5 elements - https://www.w3schools.com/TAgs/default.asp
	 *
	 * a abbr address article aside audio b bdi bdo blockquote body button canvas
	 * caption cite code colgroup datalist dd del details dfn dialog div dl dt em
	 * fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head header html i
	 * iframe ins kbd label legend li main map mark menu meter nav noscript object
	 * ol optgroup option output p picture pre progress q rp rt ruby s samp script
	 * section select small span strong style sub summary table tbody td template
	 * textarea tfoot th thead time title tr u ul var video
	 *
	 */
	rb_define_singleton_method(Berns, "a", external_a_element, -1);
	rb_define_singleton_method(Berns, "abbr", external_abbr_element, -1);
	rb_define_singleton_method(Berns, "address", external_address_element, -1);
	rb_define_singleton_method(Berns, "article", external_article_element, -1);
	rb_define_singleton_method(Berns, "aside", external_aside_element, -1);
	rb_define_singleton_method(Berns, "audio", external_audio_element, -1);
	rb_define_singleton_method(Berns, "b", external_b_element, -1);
	rb_define_singleton_method(Berns, "bdi", external_bdi_element, -1);
	rb_define_singleton_method(Berns, "bdo", external_bdo_element, -1);
	rb_define_singleton_method(Berns, "blockquote", external_blockquote_element, -1);
	rb_define_singleton_method(Berns, "body", external_body_element, -1);
	rb_define_singleton_method(Berns, "button", external_button_element, -1);
	rb_define_singleton_method(Berns, "canvas", external_canvas_element, -1);
	rb_define_singleton_method(Berns, "caption", external_caption_element, -1);
	rb_define_singleton_method(Berns, "cite", external_cite_element, -1);
	rb_define_singleton_method(Berns, "code", external_code_element, -1);
	rb_define_singleton_method(Berns, "colgroup", external_colgroup_element, -1);
	rb_define_singleton_method(Berns, "datalist", external_datalist_element, -1);
	rb_define_singleton_method(Berns, "dd", external_dd_element, -1);
	rb_define_singleton_method(Berns, "del", external_del_element, -1);
	rb_define_singleton_method(Berns, "details", external_details_element, -1);
	rb_define_singleton_method(Berns, "dfn", external_dfn_element, -1);
	rb_define_singleton_method(Berns, "dialog", external_dialog_element, -1);
	rb_define_singleton_method(Berns, "div", external_div_element, -1);
	rb_define_singleton_method(Berns, "dl", external_dl_element, -1);
	rb_define_singleton_method(Berns, "dt", external_dt_element, -1);
	rb_define_singleton_method(Berns, "em", external_em_element, -1);
	rb_define_singleton_method(Berns, "fieldset", external_fieldset_element, -1);
	rb_define_singleton_method(Berns, "figcaption", external_figcaption_element, -1);
	rb_define_singleton_method(Berns, "figure", external_figure_element, -1);
	rb_define_singleton_method(Berns, "footer", external_footer_element, -1);
	rb_define_singleton_method(Berns, "form", external_form_element, -1);
	rb_define_singleton_method(Berns, "h1", external_h1_element, -1);
	rb_define_singleton_method(Berns, "h2", external_h2_element, -1);
	rb_define_singleton_method(Berns, "h3", external_h3_element, -1);
	rb_define_singleton_method(Berns, "h4", external_h4_element, -1);
	rb_define_singleton_method(Berns, "h5", external_h5_element, -1);
	rb_define_singleton_method(Berns, "h6", external_h6_element, -1);
	rb_define_singleton_method(Berns, "head", external_head_element, -1);
	rb_define_singleton_method(Berns, "header", external_header_element, -1);
	rb_define_singleton_method(Berns, "html", external_html_element, -1);
	rb_define_singleton_method(Berns, "i", external_i_element, -1);
	rb_define_singleton_method(Berns, "iframe", external_iframe_element, -1);
	rb_define_singleton_method(Berns, "ins", external_ins_element, -1);
	rb_define_singleton_method(Berns, "kbd", external_kbd_element, -1);
	rb_define_singleton_method(Berns, "label", external_label_element, -1);
	rb_define_singleton_method(Berns, "legend", external_legend_element, -1);
	rb_define_singleton_method(Berns, "li", external_li_element, -1);
	rb_define_singleton_method(Berns, "main", external_main_element, -1);
	rb_define_singleton_method(Berns, "map", external_map_element, -1);
	rb_define_singleton_method(Berns, "mark", external_mark_element, -1);
	rb_define_singleton_method(Berns, "menu", external_menu_element, -1);
	rb_define_singleton_method(Berns, "meter", external_meter_element, -1);
	rb_define_singleton_method(Berns, "nav", external_nav_element, -1);
	rb_define_singleton_method(Berns, "noscript", external_noscript_element, -1);
	rb_define_singleton_method(Berns, "object", external_object_element, -1);
	rb_define_singleton_method(Berns, "ol", external_ol_element, -1);
	rb_define_singleton_method(Berns, "optgroup", external_optgroup_element, -1);
	rb_define_singleton_method(Berns, "option", external_option_element, -1);
	rb_define_singleton_method(Berns, "output", external_output_element, -1);
	rb_define_singleton_method(Berns, "p", external_p_element, -1);
	rb_define_singleton_method(Berns, "picture", external_picture_element, -1);
	rb_define_singleton_method(Berns, "pre", external_pre_element, -1);
	rb_define_singleton_method(Berns, "progress", external_progress_element, -1);
	rb_define_singleton_method(Berns, "q", external_q_element, -1);
	rb_define_singleton_method(Berns, "rp", external_rp_element, -1);
	rb_define_singleton_method(Berns, "rt", external_rt_element, -1);
	rb_define_singleton_method(Berns, "ruby", external_ruby_element, -1);
	rb_define_singleton_method(Berns, "s", external_s_element, -1);
	rb_define_singleton_method(Berns, "samp", external_samp_element, -1);
	rb_define_singleton_method(Berns, "script", external_script_element, -1);
	rb_define_singleton_method(Berns, "section", external_section_element, -1);
	rb_define_singleton_method(Berns, "select", external_select_element, -1);
	rb_define_singleton_method(Berns, "small", external_small_element, -1);
	rb_define_singleton_method(Berns, "span", external_span_element, -1);
	rb_define_singleton_method(Berns, "strong", external_strong_element, -1);
	rb_define_singleton_method(Berns, "style", external_style_element, -1);
	rb_define_singleton_method(Berns, "sub", external_sub_element, -1);
	rb_define_singleton_method(Berns, "summary", external_summary_element, -1);
	rb_define_singleton_method(Berns, "table", external_table_element, -1);
	rb_define_singleton_method(Berns, "tbody", external_tbody_element, -1);
	rb_define_singleton_method(Berns, "td", external_td_element, -1);
	rb_define_singleton_method(Berns, "template", external_template_element, -1);
	rb_define_singleton_method(Berns, "textarea", external_textarea_element, -1);
	rb_define_singleton_method(Berns, "tfoot", external_tfoot_element, -1);
	rb_define_singleton_method(Berns, "th", external_th_element, -1);
	rb_define_singleton_method(Berns, "thead", external_thead_element, -1);
	rb_define_singleton_method(Berns, "time", external_time_element, -1);
	rb_define_singleton_method(Berns, "title", external_title_element, -1);
	rb_define_singleton_method(Berns, "tr", external_tr_element, -1);
	rb_define_singleton_method(Berns, "u", external_u_element, -1);
	rb_define_singleton_method(Berns, "ul", external_ul_element, -1);
	rb_define_singleton_method(Berns, "var", external_var_element, -1);
	rb_define_singleton_method(Berns, "video", external_video_element, -1);
}
