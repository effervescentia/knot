open Core;

let quote = Char('"');

let rec lex_string = _ =>
  Lexers([
    Lexer(
      quote,
      Any,
      s => Result(String(String.sub(s, 1, String.length(s) - 2))),
    ),
    Lexer(Except([quote]), Any, lex_string),
  ]);

let lexer = Lexer(quote, Any, lex_string);
