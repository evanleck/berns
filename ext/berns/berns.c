#include "ruby.h"
#include "extconf.h"

/*
 * "Safe strcpy" - https://twitter.com/hyc_symas/status/1102573036534972416?s=12
*/
static char *stecpy(char *destination, const char *source, const char *end) {
  while (*source && destination < end) {
    *destination++ = *source++;
  }

  if (destination < end) {
    *destination = '\0';
  }

  return destination;
}

/*
 * Berns.escape_html is effectively a proxy to CGI.escapeHTML so we limit the
 * type to strings.
 */
static VALUE berns_escape_html(const VALUE self, VALUE string) {
  StringValue(string);

  const VALUE cgi = rb_const_get(rb_cObject, rb_intern("CGI"));
  const VALUE escape = rb_intern("escapeHTML");

  return rb_funcall(cgi, escape, 1, string);
}

static VALUE berns_to_attribute(const VALUE self, VALUE attribute, const VALUE value) {
  const VALUE empty = rb_utf8_str_new_cstr("");

  VALUE escaped;
  VALUE key;
  VALUE keys;
  VALUE length;
  VALUE rstring;
  VALUE subattr;
  VALUE subkey;
  VALUE subvalue;

  switch(TYPE(attribute)) {
    case T_STRING:
      break;
    case T_SYMBOL:
      attribute = rb_sym_to_s(attribute);
      break;
    default:
      rb_raise(rb_eTypeError, "Berns.to_attribute attributes must be Strings or Symbols.");
      break;
  }

  const char *close = "\"";
  const char *dash = "-";
  const char *equals = "=\"";
  const char *space = " ";

  const size_t clen = strlen(close);
  const size_t dlen = strlen(dash);
  const size_t eqlen = strlen(equals);
  const size_t splen = strlen(space);

  switch(TYPE(value)) {
    case T_NIL:
    case T_TRUE:
      return attribute;
    case T_FALSE:
      return empty;
    case T_HASH:
      keys = rb_funcall(value, rb_intern("keys"), 0);
      length = RARRAY_LEN(keys);

      if (length == 0) {
        return rb_utf8_str_new_cstr("");
      }

      char *substring = NULL;
      size_t size = 0;

      for (unsigned int i = 0; i < length; i++) {
        key = rb_ary_entry(keys, i);
        subvalue = rb_hash_aref(value, key);

        switch(TYPE(key)) {
          case T_NIL:
            subkey = empty;
            break;
          case T_STRING:
            subkey = key;
            break;
          case T_SYMBOL:
            subkey = rb_sym_to_s(key);
            break;
          default:
            if (substring != NULL) {
              free(substring);
            }

            rb_raise(rb_eTypeError, "Berns.to_attribute value keys must be Strings or Symbols.");
            break;
        }

        size_t total = RSTRING_LEN(attribute) + 1;

        if (RSTRING_LEN(subkey) > 0) {
          total = RSTRING_LEN(attribute) + dlen + RSTRING_LEN(subkey) + 1;
        }

        char subname[total];
        char *ptr;
        char *end = subname + sizeof(subname);

        ptr = stecpy(subname, RSTRING_PTR(attribute), end);

        if (RSTRING_LEN(subkey) > 0) {
          ptr = stecpy(ptr, dash, end);
        }

        stecpy(ptr, RSTRING_PTR(subkey), end);

        subattr = berns_to_attribute(self, rb_utf8_str_new_cstr(subname), subvalue);
        size_t subattrlen = RSTRING_LEN(subattr);

        if (i > 0) {
          size = size + splen + subattrlen;

          char *tmp = realloc(substring, size + 1);

          if (tmp == NULL) {
            rb_raise(rb_eNoMemError, "Berns.to_attribute could not allocate sufficient memory.");
          }

          substring = tmp;

          stecpy(substring + size - splen - subattrlen, space, substring + size);
          stecpy(substring + size - subattrlen, RSTRING_PTR(subattr), substring + size + 1);
        } else {
          size = size + subattrlen;
          char *tmp = realloc(substring, size + 1);

          if (tmp == NULL) {
            rb_raise(rb_eNoMemError, "Berns.to_attribute could not allocate sufficient memory.");
          }

          substring = tmp;

          stecpy(substring + size - subattrlen, RSTRING_PTR(subattr), substring + size + 1);
        }
      }

      rstring = rb_utf8_str_new_cstr(substring);
      free(substring);

      return rstring;
    default:
      switch (TYPE(value)) {
        case T_STRING:
          escaped = berns_escape_html(self, value);
          break;
        case T_SYMBOL:
          escaped = berns_escape_html(self, rb_sym_to_s(value));
          break;
        default:
          escaped = berns_escape_html(self, rb_funcall(value, rb_intern("to_s"), 0));
          break;
      }

      char string[RSTRING_LEN(attribute) + eqlen + RSTRING_LEN(escaped) + clen + 1];
      char *ptr;
      char *end = string + sizeof(string);

      ptr = stecpy(string, RSTRING_PTR(attribute), end);
      ptr = stecpy(ptr, equals, end);
      ptr = stecpy(ptr, RSTRING_PTR(escaped), end);
      stecpy(ptr, close, end);

      return rb_utf8_str_new_cstr(string);
  }
}

/* Expects a Ruby Hash as a single argument. */
static VALUE berns_to_attributes(const VALUE self, const VALUE attributes) {
  Check_Type(attributes, T_HASH);

  VALUE key;
  VALUE attribute;
  VALUE rstring;

  const VALUE keys = rb_funcall(attributes, rb_intern("keys"), 0);
  const VALUE length = RARRAY_LEN(keys);

  if (length == 0) {
    return rb_utf8_str_new_cstr("");
  }

  char *string = NULL;
  size_t size = 0; /* IN BYTES */

  const char *space = " ";
  const size_t splen = strlen(space);

  size_t alen;

  for (unsigned int i = 0; i < length; i++) {
    key = rb_ary_entry(keys, i);
    attribute = berns_to_attribute(self, key, rb_hash_aref(attributes, key));
    alen = RSTRING_LEN(attribute);

    if (i > 0) {
      char *tmp = realloc(string, size + alen + splen + 1);

      if (tmp == NULL) {
        rb_raise(rb_eNoMemError, "Berns.to_attributes could not allocate sufficient memory.");
      }

      string = tmp;

      stecpy(string + size, space, string + size + splen);
      stecpy(string + size + splen, RSTRING_PTR(attribute), string + size + splen + alen + 1);
      size = size + splen + alen;
    } else {
      char *tmp = realloc(string, size + alen + 1);

      if (tmp == NULL) {
        rb_raise(rb_eNoMemError, "Berns.to_attributes could not allocate sufficient memory.");
      }

      string = tmp;

      stecpy(string + size, RSTRING_PTR(attribute), string + size + alen + 1);
      size = size + alen;
    }
  }

  rstring = rb_utf8_str_new_cstr(string);
  free(string);

  return rstring;
}

static VALUE berns_internal_void(VALUE tag, VALUE attributes) {
  const VALUE berns = rb_const_get(rb_cObject, rb_intern("Berns"));

  switch(TYPE(tag)) {
    case T_STRING:
      break;
    case T_SYMBOL:
      tag = rb_sym_to_s(tag);
      break;
    default:
      rb_raise(rb_eTypeError, "Berns.void elements must be a String or Symbol.");
      break;
  }

  const char *open = "<";
  const char *close = ">";
  const char *space = " ";

  const size_t olen = strlen(open);
  const size_t clen = strlen(close);
  const size_t slen = strlen(space);

  size_t tlen = RSTRING_LEN(tag);
  size_t total;
  size_t alen = 0;

  if (TYPE(attributes) != T_IMEMO) {
    attributes = berns_to_attributes(berns, attributes);
    alen = RSTRING_LEN(attributes);

    if (alen > 0) {
      total = olen + tlen + slen + alen + clen + 1;
    } else {
      total = olen + tlen + clen + 1;
    }
  } else {
    total = olen + tlen + clen + 1;
  }

  char string[total];
  char *ptr, *end = string + sizeof(string);

  ptr = stecpy(string, open, end);
  ptr = stecpy(ptr, RSTRING_PTR(tag), end);

  if (TYPE(attributes) != T_IMEMO && alen > 0) {
    ptr = stecpy(ptr, space, end);
    ptr = stecpy(ptr, RSTRING_PTR(attributes), end);
  }

  stecpy(ptr, close, end);

  return rb_utf8_str_new_cstr(string);
}

static VALUE berns_internal_element(VALUE tag, VALUE attributes) {
  const VALUE berns = rb_const_get(rb_cObject, rb_intern("Berns"));

  VALUE content;

  switch(TYPE(tag)) {
    case T_STRING:
      break;
    case T_SYMBOL:
      tag = rb_sym_to_s(tag);
      break;
    default:
      rb_raise(rb_eTypeError, "Berns.element elements must be a String or Symbol.");
      break;
  }

  if (rb_block_given_p()) {
    content = rb_yield(Qnil);

    if (TYPE(content) == T_NIL || TYPE(content) == T_FALSE) {
      content = rb_utf8_str_new_cstr("");
    } else if (TYPE(content) != T_STRING) {
      content = rb_funcall(content, rb_intern("to_s"), 0);
    }
  } else {
    content = rb_utf8_str_new_cstr("");
  }

  StringValue(content);

  const char *open = "<";
  const char *close = ">";
  const char *slash = "/";
  const char *space = " ";

  const size_t olen = strlen(open);
  const size_t clen = strlen(close);
  const size_t sllen = strlen(slash);
  const size_t slen = strlen(space);

  size_t tlen = RSTRING_LEN(tag);
  size_t conlen = RSTRING_LEN(content);
  size_t total = olen + tlen + clen + conlen + olen + sllen + tlen + clen + 1;

  if (TYPE(attributes) != T_IMEMO) {
    attributes = berns_to_attributes(berns, attributes);
    total = total + slen + RSTRING_LEN(attributes);
  }

  char string[total];
  char *ptr;
  char *end = string + sizeof(string);

  ptr = stecpy(string, open, end);
  ptr = stecpy(ptr, RSTRING_PTR(tag), end);

  if (TYPE(attributes) != T_IMEMO) {
    ptr = stecpy(ptr, space, end);
    ptr = stecpy(ptr, RSTRING_PTR(attributes), end);
  }

  ptr = stecpy(ptr, close, end);
  ptr = stecpy(ptr, RSTRING_PTR(content), end);
  ptr = stecpy(ptr, open, end);
  ptr = stecpy(ptr, slash, end);
  ptr = stecpy(ptr, RSTRING_PTR(tag), end);
  stecpy(ptr, close, end);

  return rb_utf8_str_new_cstr(string);
}

static VALUE berns_void_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 1, 2);
  return berns_internal_void(argv[0], argv[1]);
}

static VALUE berns_area_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("area"), argv[0]);
}

static VALUE berns_base_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("base"), argv[0]);
}

static VALUE berns_br_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("br"), argv[0]);
}

static VALUE berns_col_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("col"), argv[0]);
}

static VALUE berns_embed_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("embed"), argv[0]);
}

static VALUE berns_hr_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("hr"), argv[0]);
}

static VALUE berns_img_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("img"), argv[0]);
}

static VALUE berns_input_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("input"), argv[0]);
}

static VALUE berns_link_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("link"), argv[0]);
}

static VALUE berns_menuitem_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("menuitem"), argv[0]);
}

static VALUE berns_meta_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("meta"), argv[0]);
}

static VALUE berns_param_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("param"), argv[0]);
}

static VALUE berns_source_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("source"), argv[0]);
}

static VALUE berns_track_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("track"), argv[0]);
}

static VALUE berns_wbr_element(int argc, VALUE *argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_void(rb_utf8_str_new_cstr("wbr"), argv[0]);
}

static VALUE berns_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 1, 2);
  return berns_internal_element(argv[0], argv[1]);
}

static VALUE berns_a_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("a"), argv[0]);
}

static VALUE berns_abbr_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("abbr"), argv[0]);
}

static VALUE berns_address_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("address"), argv[0]);
}

static VALUE berns_article_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("article"), argv[0]);
}

static VALUE berns_aside_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("aside"), argv[0]);
}

static VALUE berns_audio_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("audio"), argv[0]);
}

static VALUE berns_b_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("b"), argv[0]);
}

static VALUE berns_bdi_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("bdi"), argv[0]);
}

static VALUE berns_bdo_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("bdo"), argv[0]);
}

static VALUE berns_blockquote_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("blockquote"), argv[0]);
}

static VALUE berns_body_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("body"), argv[0]);
}

static VALUE berns_button_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("button"), argv[0]);
}

static VALUE berns_canvas_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("canvas"), argv[0]);
}

static VALUE berns_caption_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("caption"), argv[0]);
}

static VALUE berns_cite_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("cite"), argv[0]);
}

static VALUE berns_code_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("code"), argv[0]);
}

static VALUE berns_colgroup_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("colgroup"), argv[0]);
}

static VALUE berns_datalist_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("datalist"), argv[0]);
}

static VALUE berns_dd_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("dd"), argv[0]);
}

static VALUE berns_del_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("del"), argv[0]);
}

static VALUE berns_details_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("details"), argv[0]);
}

static VALUE berns_dfn_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("dfn"), argv[0]);
}

static VALUE berns_dialog_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("dialog"), argv[0]);
}

static VALUE berns_div_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("div"), argv[0]);
}

static VALUE berns_dl_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("dl"), argv[0]);
}

static VALUE berns_dt_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("dt"), argv[0]);
}

static VALUE berns_em_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("em"), argv[0]);
}

static VALUE berns_fieldset_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("fieldset"), argv[0]);
}

static VALUE berns_figcaption_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("figcaption"), argv[0]);
}

static VALUE berns_figure_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("figure"), argv[0]);
}

static VALUE berns_footer_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("footer"), argv[0]);
}

static VALUE berns_form_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("form"), argv[0]);
}

static VALUE berns_h1_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h1"), argv[0]);
}

static VALUE berns_h2_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h2"), argv[0]);
}

static VALUE berns_h3_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h3"), argv[0]);
}

static VALUE berns_h4_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h4"), argv[0]);
}

static VALUE berns_h5_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h5"), argv[0]);
}

static VALUE berns_h6_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("h6"), argv[0]);
}

static VALUE berns_head_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("head"), argv[0]);
}

static VALUE berns_header_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("header"), argv[0]);
}

static VALUE berns_html_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("html"), argv[0]);
}

static VALUE berns_i_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("i"), argv[0]);
}

static VALUE berns_iframe_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("iframe"), argv[0]);
}

static VALUE berns_ins_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("ins"), argv[0]);
}

static VALUE berns_kbd_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("kbd"), argv[0]);
}

static VALUE berns_label_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("label"), argv[0]);
}

static VALUE berns_legend_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("legend"), argv[0]);
}

static VALUE berns_li_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("li"), argv[0]);
}

static VALUE berns_main_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("main"), argv[0]);
}

static VALUE berns_map_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("map"), argv[0]);
}

static VALUE berns_mark_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("mark"), argv[0]);
}

static VALUE berns_menu_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("menu"), argv[0]);
}

static VALUE berns_meter_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("meter"), argv[0]);
}

static VALUE berns_nav_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("nav"), argv[0]);
}

static VALUE berns_noscript_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("noscript"), argv[0]);
}

static VALUE berns_object_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("object"), argv[0]);
}

static VALUE berns_ol_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("ol"), argv[0]);
}

static VALUE berns_optgroup_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("optgroup"), argv[0]);
}

static VALUE berns_option_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("option"), argv[0]);
}

static VALUE berns_output_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("output"), argv[0]);
}

static VALUE berns_p_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("p"), argv[0]);
}

static VALUE berns_picture_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("picture"), argv[0]);
}

static VALUE berns_pre_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("pre"), argv[0]);
}

static VALUE berns_progress_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("progress"), argv[0]);
}

static VALUE berns_q_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("q"), argv[0]);
}

static VALUE berns_rp_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("rp"), argv[0]);
}

static VALUE berns_rt_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("rt"), argv[0]);
}

static VALUE berns_ruby_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("ruby"), argv[0]);
}

static VALUE berns_s_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("s"), argv[0]);
}

static VALUE berns_samp_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("samp"), argv[0]);
}

static VALUE berns_script_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("script"), argv[0]);
}

static VALUE berns_section_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("section"), argv[0]);
}

static VALUE berns_select_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("select"), argv[0]);
}

static VALUE berns_small_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("small"), argv[0]);
}

static VALUE berns_span_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("span"), argv[0]);
}

static VALUE berns_strong_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("strong"), argv[0]);
}

static VALUE berns_style_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("style"), argv[0]);
}

static VALUE berns_sub_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("sub"), argv[0]);
}

static VALUE berns_summary_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("summary"), argv[0]);
}

static VALUE berns_table_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("table"), argv[0]);
}

static VALUE berns_tbody_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("tbody"), argv[0]);
}

static VALUE berns_td_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("td"), argv[0]);
}

static VALUE berns_template_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("template"), argv[0]);
}

static VALUE berns_textarea_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("textarea"), argv[0]);
}

static VALUE berns_tfoot_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("tfoot"), argv[0]);
}

static VALUE berns_th_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("th"), argv[0]);
}

static VALUE berns_thead_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("thead"), argv[0]);
}

static VALUE berns_time_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("time"), argv[0]);
}

static VALUE berns_title_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("title"), argv[0]);
}

static VALUE berns_tr_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("tr"), argv[0]);
}

static VALUE berns_u_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("u"), argv[0]);
}

static VALUE berns_ul_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("ul"), argv[0]);
}

static VALUE berns_var_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("var"), argv[0]);
}

static VALUE berns_video_element(int argc, VALUE* argv, VALUE self) {
  rb_check_arity(argc, 0, 1);
  return berns_internal_element(rb_utf8_str_new_cstr("video"), argv[0]);
}

void Init_berns() {
  VALUE Berns = rb_define_module("Berns");

  rb_define_singleton_method(Berns, "element", berns_element, -1);
  rb_define_singleton_method(Berns, "escape_html", berns_escape_html, 1);
  rb_define_singleton_method(Berns, "to_attribute", berns_to_attribute, 2);
  rb_define_singleton_method(Berns, "to_attributes", berns_to_attributes, 1);
  rb_define_singleton_method(Berns, "void", berns_void_element, -1);

  /*
   * List of void elements - http://xahlee.info/js/html5_non-closing_tag.html
   *
   *   area base br col embed hr img input link menuitem meta param source track wbr
   *
   */
  rb_define_singleton_method(Berns, "area", berns_area_element, -1);
  rb_define_singleton_method(Berns, "base", berns_base_element, -1);
  rb_define_singleton_method(Berns, "br", berns_br_element, -1);
  rb_define_singleton_method(Berns, "col", berns_col_element, -1);
  rb_define_singleton_method(Berns, "embed", berns_embed_element, -1);
  rb_define_singleton_method(Berns, "hr", berns_hr_element, -1);
  rb_define_singleton_method(Berns, "img", berns_img_element, -1);
  rb_define_singleton_method(Berns, "input", berns_input_element, -1);
  rb_define_singleton_method(Berns, "link", berns_link_element, -1);
  rb_define_singleton_method(Berns, "menuitem", berns_menuitem_element, -1);
  rb_define_singleton_method(Berns, "meta", berns_meta_element, -1);
  rb_define_singleton_method(Berns, "param", berns_param_element, -1);
  rb_define_singleton_method(Berns, "source", berns_source_element, -1);
  rb_define_singleton_method(Berns, "track", berns_track_element, -1);
  rb_define_singleton_method(Berns, "wbr", berns_wbr_element, -1);

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
  rb_define_singleton_method(Berns, "a", berns_a_element, -1);
  rb_define_singleton_method(Berns, "abbr", berns_abbr_element, -1);
  rb_define_singleton_method(Berns, "address", berns_address_element, -1);
  rb_define_singleton_method(Berns, "article", berns_article_element, -1);
  rb_define_singleton_method(Berns, "aside", berns_aside_element, -1);
  rb_define_singleton_method(Berns, "audio", berns_audio_element, -1);
  rb_define_singleton_method(Berns, "b", berns_b_element, -1);
  rb_define_singleton_method(Berns, "bdi", berns_bdi_element, -1);
  rb_define_singleton_method(Berns, "bdo", berns_bdo_element, -1);
  rb_define_singleton_method(Berns, "blockquote", berns_blockquote_element, -1);
  rb_define_singleton_method(Berns, "body", berns_body_element, -1);
  rb_define_singleton_method(Berns, "button", berns_button_element, -1);
  rb_define_singleton_method(Berns, "canvas", berns_canvas_element, -1);
  rb_define_singleton_method(Berns, "caption", berns_caption_element, -1);
  rb_define_singleton_method(Berns, "cite", berns_cite_element, -1);
  rb_define_singleton_method(Berns, "code", berns_code_element, -1);
  rb_define_singleton_method(Berns, "colgroup", berns_colgroup_element, -1);
  rb_define_singleton_method(Berns, "datalist", berns_datalist_element, -1);
  rb_define_singleton_method(Berns, "dd", berns_dd_element, -1);
  rb_define_singleton_method(Berns, "del", berns_del_element, -1);
  rb_define_singleton_method(Berns, "details", berns_details_element, -1);
  rb_define_singleton_method(Berns, "dfn", berns_dfn_element, -1);
  rb_define_singleton_method(Berns, "dialog", berns_dialog_element, -1);
  rb_define_singleton_method(Berns, "div", berns_div_element, -1);
  rb_define_singleton_method(Berns, "dl", berns_dl_element, -1);
  rb_define_singleton_method(Berns, "dt", berns_dt_element, -1);
  rb_define_singleton_method(Berns, "em", berns_em_element, -1);
  rb_define_singleton_method(Berns, "fieldset", berns_fieldset_element, -1);
  rb_define_singleton_method(Berns, "figcaption", berns_figcaption_element, -1);
  rb_define_singleton_method(Berns, "figure", berns_figure_element, -1);
  rb_define_singleton_method(Berns, "footer", berns_footer_element, -1);
  rb_define_singleton_method(Berns, "form", berns_form_element, -1);
  rb_define_singleton_method(Berns, "h1", berns_h1_element, -1);
  rb_define_singleton_method(Berns, "h2", berns_h2_element, -1);
  rb_define_singleton_method(Berns, "h3", berns_h3_element, -1);
  rb_define_singleton_method(Berns, "h4", berns_h4_element, -1);
  rb_define_singleton_method(Berns, "h5", berns_h5_element, -1);
  rb_define_singleton_method(Berns, "h6", berns_h6_element, -1);
  rb_define_singleton_method(Berns, "head", berns_head_element, -1);
  rb_define_singleton_method(Berns, "header", berns_header_element, -1);
  rb_define_singleton_method(Berns, "html", berns_html_element, -1);
  rb_define_singleton_method(Berns, "i", berns_i_element, -1);
  rb_define_singleton_method(Berns, "iframe", berns_iframe_element, -1);
  rb_define_singleton_method(Berns, "ins", berns_ins_element, -1);
  rb_define_singleton_method(Berns, "kbd", berns_kbd_element, -1);
  rb_define_singleton_method(Berns, "label", berns_label_element, -1);
  rb_define_singleton_method(Berns, "legend", berns_legend_element, -1);
  rb_define_singleton_method(Berns, "li", berns_li_element, -1);
  rb_define_singleton_method(Berns, "main", berns_main_element, -1);
  rb_define_singleton_method(Berns, "map", berns_map_element, -1);
  rb_define_singleton_method(Berns, "mark", berns_mark_element, -1);
  rb_define_singleton_method(Berns, "menu", berns_menu_element, -1);
  rb_define_singleton_method(Berns, "meter", berns_meter_element, -1);
  rb_define_singleton_method(Berns, "nav", berns_nav_element, -1);
  rb_define_singleton_method(Berns, "noscript", berns_noscript_element, -1);
  rb_define_singleton_method(Berns, "object", berns_object_element, -1);
  rb_define_singleton_method(Berns, "ol", berns_ol_element, -1);
  rb_define_singleton_method(Berns, "optgroup", berns_optgroup_element, -1);
  rb_define_singleton_method(Berns, "option", berns_option_element, -1);
  rb_define_singleton_method(Berns, "output", berns_output_element, -1);
  rb_define_singleton_method(Berns, "p", berns_p_element, -1);
  rb_define_singleton_method(Berns, "picture", berns_picture_element, -1);
  rb_define_singleton_method(Berns, "pre", berns_pre_element, -1);
  rb_define_singleton_method(Berns, "progress", berns_progress_element, -1);
  rb_define_singleton_method(Berns, "q", berns_q_element, -1);
  rb_define_singleton_method(Berns, "rp", berns_rp_element, -1);
  rb_define_singleton_method(Berns, "rt", berns_rt_element, -1);
  rb_define_singleton_method(Berns, "ruby", berns_ruby_element, -1);
  rb_define_singleton_method(Berns, "s", berns_s_element, -1);
  rb_define_singleton_method(Berns, "samp", berns_samp_element, -1);
  rb_define_singleton_method(Berns, "script", berns_script_element, -1);
  rb_define_singleton_method(Berns, "section", berns_section_element, -1);
  rb_define_singleton_method(Berns, "select", berns_select_element, -1);
  rb_define_singleton_method(Berns, "small", berns_small_element, -1);
  rb_define_singleton_method(Berns, "span", berns_span_element, -1);
  rb_define_singleton_method(Berns, "strong", berns_strong_element, -1);
  rb_define_singleton_method(Berns, "style", berns_style_element, -1);
  rb_define_singleton_method(Berns, "sub", berns_sub_element, -1);
  rb_define_singleton_method(Berns, "summary", berns_summary_element, -1);
  rb_define_singleton_method(Berns, "table", berns_table_element, -1);
  rb_define_singleton_method(Berns, "tbody", berns_tbody_element, -1);
  rb_define_singleton_method(Berns, "td", berns_td_element, -1);
  rb_define_singleton_method(Berns, "template", berns_template_element, -1);
  rb_define_singleton_method(Berns, "textarea", berns_textarea_element, -1);
  rb_define_singleton_method(Berns, "tfoot", berns_tfoot_element, -1);
  rb_define_singleton_method(Berns, "th", berns_th_element, -1);
  rb_define_singleton_method(Berns, "thead", berns_thead_element, -1);
  rb_define_singleton_method(Berns, "time", berns_time_element, -1);
  rb_define_singleton_method(Berns, "title", berns_title_element, -1);
  rb_define_singleton_method(Berns, "tr", berns_tr_element, -1);
  rb_define_singleton_method(Berns, "u", berns_u_element, -1);
  rb_define_singleton_method(Berns, "ul", berns_ul_element, -1);
  rb_define_singleton_method(Berns, "var", berns_var_element, -1);
  rb_define_singleton_method(Berns, "video", berns_video_element, -1);
}
