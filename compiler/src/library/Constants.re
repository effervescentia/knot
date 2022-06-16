/**
 Symbols, glyphs and keywords used in the grammar.
 */

let root_dir = "@/";
let file_extension = ".kn";

let private_prefix = "_";

/**
 single character symbols
 */
module Character = {
  let back_slash = '\\';
  let eol = '\n';
  let quote = '"';
  let period = '.';
  let comma = ',';
  let octothorpe = '#';
  let underscore = '_';
  let colon = ':';
  let semicolon = ';';
  let equal_sign = '=';
  let vertical_bar = '|';

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

/**
 multi-character symbols
 */
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

  let list_type_suffix = "[]";
};

/**
 language keywords
 */
module Keyword = {
  let import = "import";
  let from = "from";
  let main = "main";
  let const = "const";
  let enum = "enum";
  let func = "func";
  let view = "view";
  let style = "style";
  let let_ = "let";
  let nil = "nil";
  let true_ = "true";
  let false_ = "false";
  let as_ = "as";
  let decl = "decl";
  let type_ = "type";
  let module_ = "module";

  /* primitive types */
  let boolean = "boolean";
  let integer = "integer";
  let float = "float";
  let string = "string";
  let element = "element";

  /**
   reserved keywords cannot be used as the name of a variable
   */
  let reserved = [
    import,
    from,
    main,
    const,
    enum,
    func,
    view,
    style,
    let_,
    nil,
    true_,
    false_,
    as_,
  ];
};
