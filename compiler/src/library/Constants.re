/**
 Symbols, glyphs and keywords used in the grammar.
 */

let root_dir = "@/";
let file_extension = ".kn";

let private_prefix = "_";

module Character = {
  let back_slash = '\\';
  let eol = '\n';
  let quote = '"';
  let period = '.';
  let comma = ',';
  let octothorp = '#';
  let underscore = '_';
  let semicolon = ';';
  let equal_sign = '=';

  let exclamation_mark = '!';
  let caret = '^';

  let asterisk = '*';
  let forward_slash = '/';
  let plus_sign = '+';
  let minus_sign = '-';

  let open_paren = '(';
  let close_paren = ')';

  let open_brace = '{';
  let close_brace = '}';

  let open_bracket = '[';
  let close_bracket = ']';

  let open_chevron = '<';
  let close_chevron = '>';
};

module Glyph = {
  let logical_and = "&&";
  let logical_or = "||";

  let less_or_eql = "<=";
  let less_than = "<";
  let greater_or_eql = ">=";
  let greater_than = ">";

  let equality = "==";
  let inequality = "!=";

  let open_fragment = "<>";
  let close_fragment = "</>";

  let self_close_tag = "/>";
  let open_end_tag = "</";

  let lambda = "->";
};

module Keyword = {
  /* declarations */
  let const = "const";
  let type_ = "type";
  let func = "func";
  let let_ = "let";

  /* utility */
  let main = "main";
  let as_ = "as";

  /* import */
  let import = "import";
  let from = "from";

  /* primitives */
  let nil = "nil";
  let true_ = "true";
  let false_ = "false";

  /* type primitives */
  let bool = "bool";
  let int = "int";
  let float = "float";
  let number = "number";
  let string = "string";
  let element = "Element";

  /**
   * keywords which cannot be used as the name of an indentifier
   */
  let reserved = [const, type_, func, let_, nil, true_, false_, as_];
};
