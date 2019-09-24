open Core;

let _quote = Char('"');

let rec lex_string = _ =>
  Lexers([
    Lexer(
      _quote,
      Any,
      s => Result(String(String.sub(s, 1, String.length(s) - 2))),
    ),
    Lexer(Except([_quote]), Any, lex_string),
  ]);

let lexer = Lexer(_quote, Any, lex_string);
