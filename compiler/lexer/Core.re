include Knot.Globals;
include Knot.Token;

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

let rec (===>) = (s, t) => {
  let next = _ =>
    if (String.length(s) == 1) {
      Result(t);
    } else {
      String.sub(s, 1, String.length(s) - 1) ===> t;
    };

  Lexer(Char(s.[0]), Any, next);
};
