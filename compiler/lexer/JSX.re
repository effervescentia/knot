open Core;

let left_brace = Char('{');
let left_chev = Char('<');

module Content = {
  let rec lex_text_node =
    Lexers([
      Lexer(
        Except([left_brace, left_chev]),
        Except([left_brace, left_chev]),
        _ => lex_text_node,
      ),
      Lexer(
        Except([left_brace, left_chev]),
        Either([left_brace, left_chev]),
        s => Result(JSXTextNode(s)),
      ),
    ]);

  let lexer =
    Lexers([
      Character.jsx_content_lexer,
      Pattern.jsx_content_lexer,
      lex_text_node,
    ]);
};

module StartTag = {
  let lexer =
    Lexers([
      Identifier.lexer(~reserved=[], ()),
      Text.lexer,
      Pattern.jsx_start_tag_lexer,
      Character.jsx_start_tag_lexer,
    ]);
};

module EndTag = {
  let lexer =
    Lexers([
      Identifier.lexer(~reserved=[], ()),
      Character.jsx_end_tag_lexer,
    ]);
};
