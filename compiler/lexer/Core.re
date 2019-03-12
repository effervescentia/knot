include Knot.Globals;
include Knot.Token;
include Exception;

type context =
  | Normal
  | JSXStartTag
  | JSXEndTag
  | JSXContent;

type lex_match =
  | Any
  | Alpha
  | Numeric
  | AlphaNumeric
  | Char(char)
  | Token(string)
  | Either(list(lex_match))
  | Except(list(lex_match));

type lex_result('a) =
  | Lexers(list(lex_result('a)))
  | Lexer(lex_match, lex_match, string => lex_result('a))
  | Result('a);

let newline = Char('\n');
