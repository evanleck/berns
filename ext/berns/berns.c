#include "hescape.h"
#include "ruby.h"
#include "sds.h"

static const char *attr_close = "\"";
static const char *attr_equals = "=\"";
static const char *dash = "-";
static const char *space = " ";
static const char *tag_open = "<";
static const char *tag_close = ">";
static const char *slash = "/";


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
#define VOID_ELEMENT(element_name, length) \
  static VALUE external_##element_name##_element(int argc, VALUE *argv, RB_UNUSED_VAR(VALUE self)) { \
    rb_check_arity(argc, 0, 1); \
    \
    sds string = void_element(#element_name, length, argv[0]); \
    VALUE rstring = rb_utf8_str_new_cstr(string); \
    sdsfree(string); \
    \
    return rstring; \
  }

/*
 * Macro to define a "dynamic" function that generates a standard element.
 */
#define STANDARD_ELEMENT(element_name, length) \
  static VALUE external_##element_name##_element(int argc, VALUE *argv, RB_UNUSED_VAR(VALUE self)) { \
    rb_check_arity(argc, 0, 1); \
    \
    CONTENT_FROM_BLOCK \
    sds string = element(#element_name, length, RSTRING_PTR(content), RSTRING_LEN(content), argv[0]); \
    VALUE rstring = rb_utf8_str_new_cstr(string); \
    sdsfree(string); \
    \
    return rstring; \
  }


/*
 * The external API for Berns.sanitize
 *
 *   string should be a string or nil, anything else will raise an error.
 *
 */
static VALUE external_sanitize(RB_UNUSED_VAR(VALUE self), VALUE string) {
  if (TYPE(string) == T_NIL) {
    return Qnil;
  }

  StringValue(string);

  size_t slen = RSTRING_LEN(string);
  char *str = RSTRING_PTR(string);

  char dest[slen + 1];

  unsigned int index = 0;
  unsigned int open = 0;
  unsigned int modified = 0;
  unsigned int entity = 0;

  for (unsigned int i = 0; i < slen; i++) {
    if (str[i] == '<') {
      open = 1;
      modified = 1;
    } else if (str[i] == '>') {
      open = 0;
    } else if (str[i] == '&') {
      entity = 1;
      modified = 1;
    } else if (str[i] == ';') {
      entity = 0;
    } else if (!open && !entity) {
      dest[index++] = str[i];
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
 *   string should be a string, anything else will raise an error.
 *
 */
static VALUE external_escape_html(RB_UNUSED_VAR(VALUE self), VALUE string) {
  StringValue(string);

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
 * Takes a string attribute name and value pair and converts them into a string ready to use in HTML
 *
 *   "class" + "bg-primary" => 'class="bg-primary"'
*/
static sds string_value_to_attribute(const sds attr, const char *value, const size_t vallen) {
  if (vallen == 0) {
    return sdsdup(attr);
  } else {
    uint8_t *edest = NULL;
    size_t esclen = hesc_escape_html(&edest, value, vallen);

    sds string = sdsdup(attr);
    string = sdscat(string, attr_equals);
    string = sdscat(string, edest);
    string = sdscat(string, attr_close);

    if (esclen > vallen) {
      free(edest);
    }

    return string;
  }
}

static sds hash_value_to_attribute(const sds attr, VALUE value) {
  if (TYPE(value) == T_IMEMO) {
    return sdsempty();
  }

  Check_Type(value, T_HASH);

  if (RHASH_SIZE(value) == 0) {
    return sdsempty();
  }

  VALUE subkey;
  VALUE subvalue;

  const VALUE keys = rb_funcall(value, rb_intern("keys"), 0);
  const VALUE length = RARRAY_LEN(keys);

  sds destination = sdsempty();
  size_t attrlen = sdslen(attr);

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
        sdsfree(destination);
        rb_raise(rb_eTypeError, "Berns.to_attribute value keys must be Strings, Symbols, or nil.");
        break;
    }

    sds subattr = sdsempty();
    size_t subkey_len = RSTRING_LEN(subkey);

    if (attrlen > 0) {
      subattr = sdscat(subattr, attr);
    }

    if (attrlen > 0 && subkey_len > 0) {
      subattr = sdscat(subattr, dash);
    }

    subattr = sdscat(subattr, RSTRING_PTR(subkey));

    /* char *combined; */
    sds combined = sdsempty();

    switch(TYPE(subvalue)) {
      case T_FALSE:
        break;

      case T_NIL:
        /* Fall through. */
      case T_TRUE:
        combined = sdsdup(subattr);
        break;

      case T_STRING:
        combined = string_value_to_attribute(subattr, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
        break;

      case T_SYMBOL:
        subvalue = rb_sym2str(subvalue);
        combined = string_value_to_attribute(subattr, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
        break;

      case T_HASH:
        combined = hash_value_to_attribute(subattr, subvalue);
        break;

      default:
        subvalue = rb_funcall(subvalue, rb_intern("to_s"), 0);
        combined = string_value_to_attribute(subattr, RSTRING_PTR(subvalue), RSTRING_LEN(subvalue));
        break;
    }

    if (i > 0) {
      destination = sdscat(destination, space);
    }

    destination = sdscat(destination, combined);

    sdsfree(subattr);
    sdsfree(combined);
  }

  return destination;
}

/*
 * Convert an attribute name and value into a string.
 */
static sds to_attribute(VALUE attr, VALUE value) {
  switch(TYPE(attr)) {
    case T_SYMBOL:
      attr = rb_sym2str(attr);
      break;
    default:
      break;
  }

  StringValue(attr);

  sds val = sdsempty();
  sds atr = sdsnewlen(RSTRING_PTR(attr), RSTRING_LEN(attr));

  VALUE str;

  switch(TYPE(value)) {
    case T_NIL:
      /* Fall through. */
    case T_TRUE:
      val = sdsdup(atr);
      break;
    case T_FALSE:
      /* sdsempty() */
      break;
    case T_HASH:
      val = hash_value_to_attribute(atr, value);
      break;
    case T_STRING:
      val = string_value_to_attribute(atr, RSTRING_PTR(value), RSTRING_LEN(value));
      break;
    case T_SYMBOL:
      str = rb_sym2str(value);
      val = string_value_to_attribute(atr, RSTRING_PTR(str), RSTRING_LEN(str));
      break;
    default:
      str = rb_funcall(value, rb_intern("to_s"), 0);
      val = string_value_to_attribute(atr, RSTRING_PTR(str), RSTRING_LEN(str));
      break;
  }

  sdsfree(atr);

  return val;
}

/*
 * The external API for Berns.to_attribute.
 *
 *   attr should be either a symbol or string, otherwise an error is raised.
 *   value can be anything to responds to #to_s
 *
 */
static VALUE external_to_attribute(RB_UNUSED_VAR(VALUE self), VALUE attr, VALUE value) {
  switch(TYPE(attr)) {
    case T_SYMBOL:
      attr = rb_sym2str(attr);
      break;
    default:
      break;
  }

  StringValue(attr);

  sds val = to_attribute(attr, value);
  VALUE rstring = rb_utf8_str_new_cstr(val);
  sdsfree(val);

  return rstring;
}

/*
 * The external API for Berns.to_attributes.
 *
 *   attributes should be a hash, otherwise an error is raised.
 *
 */
static VALUE external_to_attributes(RB_UNUSED_VAR(VALUE self), VALUE attributes) {
  Check_Type(attributes, T_HASH);

  if (RHASH_SIZE(attributes) == 0) {
    return rb_utf8_str_new_cstr("");
  }

  sds empty = sdsempty();
  sds attrs = hash_value_to_attribute(empty, attributes);
  sdsfree(empty);

  VALUE rstring = rb_utf8_str_new_cstr(attrs);
  sdsfree(attrs);

  return rstring;
}

static sds void_element(const char *tag, size_t tlen, VALUE attributes) {
  sds empty = sdsempty();
  sds attrs = hash_value_to_attribute(empty, attributes);
  sdsfree(empty);
  size_t alen = sdslen(attrs);

  sds elm = sdsnew(tag_open);
  elm = sdscat(elm, tag);

  if (alen > 0) {
    elm = sdscat(elm, space);
    elm = sdscat(elm, attrs);
  }

  sdsfree(attrs);

  elm = sdscat(elm, tag_close);

  return elm;
}

/*
 * The external API for Berns.void.
 *
 *   The first argument should be a string or symbol, otherwise an error is raised.
 *   The second argument must be a hash if present.
 *
 */
static VALUE external_void_element(int argc, VALUE *arguments, RB_UNUSED_VAR(VALUE self)) {
  rb_check_arity(argc, 1, 2);

  VALUE tag = arguments[0];
  VALUE attributes = arguments[1];

  if (TYPE(tag) == T_SYMBOL) {
    tag = rb_sym2str(tag);
  }

  StringValue(tag);

  sds string = void_element(RSTRING_PTR(tag), RSTRING_LEN(tag), attributes);
  VALUE rstring = rb_utf8_str_new_cstr(string);
  sdsfree(string);

  return rstring;
}

static sds element(const char *tag, size_t tlen, char *content, size_t conlen, VALUE attributes) {
  sds empty = sdsempty();
  sds attrs = hash_value_to_attribute(empty, attributes);
  sdsfree(empty);
  size_t alen = sdslen(attrs);

  sds dest = sdsnew(tag_open);
  dest = sdscat(dest, tag);

  if (alen > 0) {
    dest = sdscat(dest, space);
    dest = sdscat(dest, attrs);
  }

  dest = sdscat(dest, tag_close);

  if (conlen > 0) {
    dest = sdscat(dest, content);
  }

  dest = sdscat(dest, tag_open);
  dest = sdscat(dest, slash);
  dest = sdscat(dest, tag);
  dest = sdscat(dest, tag_close);

  sdsfree(attrs);

  return dest;
}

/*
 * The external API for Berns.element.
 *
 *   The first argument should be a string or symbol, otherwise an error is raised.
 *   The second argument must be a hash if present.
 *   An optional block can be given which will used as the contents of the element.
 *
 */
static VALUE external_element(int argc, VALUE *arguments, RB_UNUSED_VAR(VALUE self)) {
  rb_check_arity(argc, 1, 2);

  VALUE tag = arguments[0];
  VALUE attributes = arguments[1];

  if (TYPE(tag) == T_SYMBOL) {
    tag = rb_sym2str(tag);
  }

  StringValue(tag);

  CONTENT_FROM_BLOCK

  sds string = element(RSTRING_PTR(tag), RSTRING_LEN(tag), RSTRING_PTR(content), RSTRING_LEN(content), attributes);
  VALUE rstring = rb_utf8_str_new_cstr(string);
  sdsfree(string);

  return rstring;
}

VOID_ELEMENT(area, 4)
VOID_ELEMENT(base, 4)
VOID_ELEMENT(br, 2)
VOID_ELEMENT(col, 3)
VOID_ELEMENT(embed, 5)
VOID_ELEMENT(hr, 2)
VOID_ELEMENT(img, 3)
VOID_ELEMENT(input, 5)
VOID_ELEMENT(link, 4)
VOID_ELEMENT(menuitem, 8)
VOID_ELEMENT(meta, 4)
VOID_ELEMENT(param, 5)
VOID_ELEMENT(source, 6)
VOID_ELEMENT(track, 5)
VOID_ELEMENT(wbr, 3)

STANDARD_ELEMENT(a, 1)
STANDARD_ELEMENT(abbr, 4)
STANDARD_ELEMENT(address, 7)
STANDARD_ELEMENT(article, 7)
STANDARD_ELEMENT(aside, 5)
STANDARD_ELEMENT(audio, 5)
STANDARD_ELEMENT(b, 1)
STANDARD_ELEMENT(bdi, 3)
STANDARD_ELEMENT(bdo, 3)
STANDARD_ELEMENT(blockquote, 10)
STANDARD_ELEMENT(body, 4)
STANDARD_ELEMENT(button, 6)
STANDARD_ELEMENT(canvas, 6)
STANDARD_ELEMENT(caption, 7)
STANDARD_ELEMENT(cite, 4)
STANDARD_ELEMENT(code, 4)
STANDARD_ELEMENT(colgroup, 8)
STANDARD_ELEMENT(datalist, 8)
STANDARD_ELEMENT(dd, 2)
STANDARD_ELEMENT(del, 3)
STANDARD_ELEMENT(details, 7)
STANDARD_ELEMENT(dfn, 3)
STANDARD_ELEMENT(dialog, 6)
STANDARD_ELEMENT(div, 3)
STANDARD_ELEMENT(dl, 2)
STANDARD_ELEMENT(dt, 2)
STANDARD_ELEMENT(em, 2)
STANDARD_ELEMENT(fieldset, 8)
STANDARD_ELEMENT(figcaption, 10)
STANDARD_ELEMENT(figure, 6)
STANDARD_ELEMENT(footer, 6)
STANDARD_ELEMENT(form, 4)
STANDARD_ELEMENT(h1, 2)
STANDARD_ELEMENT(h2, 2)
STANDARD_ELEMENT(h3, 2)
STANDARD_ELEMENT(h4, 2)
STANDARD_ELEMENT(h5, 2)
STANDARD_ELEMENT(h6, 2)
STANDARD_ELEMENT(head, 4)
STANDARD_ELEMENT(header, 6)
STANDARD_ELEMENT(html, 4)
STANDARD_ELEMENT(i, 1)
STANDARD_ELEMENT(iframe, 6)
STANDARD_ELEMENT(ins, 3)
STANDARD_ELEMENT(kbd, 3)
STANDARD_ELEMENT(label, 5)
STANDARD_ELEMENT(legend, 6)
STANDARD_ELEMENT(li, 2)
STANDARD_ELEMENT(main, 4)
STANDARD_ELEMENT(map, 3)
STANDARD_ELEMENT(mark, 4)
STANDARD_ELEMENT(menu, 4)
STANDARD_ELEMENT(meter, 5)
STANDARD_ELEMENT(nav, 3)
STANDARD_ELEMENT(noscript, 8)
STANDARD_ELEMENT(object, 6)
STANDARD_ELEMENT(ol, 2)
STANDARD_ELEMENT(optgroup, 8)
STANDARD_ELEMENT(option, 6)
STANDARD_ELEMENT(output, 6)
STANDARD_ELEMENT(p, 1)
STANDARD_ELEMENT(picture, 7)
STANDARD_ELEMENT(pre, 3)
STANDARD_ELEMENT(progress, 8)
STANDARD_ELEMENT(q, 1)
STANDARD_ELEMENT(rp, 2)
STANDARD_ELEMENT(rt, 2)
STANDARD_ELEMENT(ruby, 4)
STANDARD_ELEMENT(s, 1)
STANDARD_ELEMENT(samp, 4)
STANDARD_ELEMENT(script, 6)
STANDARD_ELEMENT(section, 7)
STANDARD_ELEMENT(select, 6)
STANDARD_ELEMENT(small, 5)
STANDARD_ELEMENT(span, 4)
STANDARD_ELEMENT(strong, 6)
STANDARD_ELEMENT(style, 5)
STANDARD_ELEMENT(sub, 3)
STANDARD_ELEMENT(summary, 7)
STANDARD_ELEMENT(table, 5)
STANDARD_ELEMENT(tbody, 5)
STANDARD_ELEMENT(td, 2)
STANDARD_ELEMENT(template, 8)
STANDARD_ELEMENT(textarea, 8)
STANDARD_ELEMENT(tfoot, 5)
STANDARD_ELEMENT(th, 2)
STANDARD_ELEMENT(thead, 5)
STANDARD_ELEMENT(time, 4)
STANDARD_ELEMENT(title, 5)
STANDARD_ELEMENT(tr, 2)
STANDARD_ELEMENT(u, 1)
STANDARD_ELEMENT(ul, 2)
STANDARD_ELEMENT(var, 3)
STANDARD_ELEMENT(video, 5)

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
   *   area base br col embed hr img input link menuitem meta param source track wbr
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
   *   a abbr address article aside audio b bdi bdo blockquote body button
   *   canvas caption cite code colgroup datalist dd del details dfn dialog div
   *   dl dt em fieldset figcaption figure footer form h1 h2 h3 h4 h5 h6 head
   *   header html i iframe ins kbd label legend li main map mark menu meter nav
   *   noscript object ol optgroup option output p picture pre progress q rp rt
   *   ruby s samp script section select small span strong style sub summary
   *   table tbody td template textarea tfoot th thead time title tr u ul var
   *   video
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
