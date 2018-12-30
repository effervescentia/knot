open Core;
open Knot.Token;

exception UnclosedString;

let quote = Char('"');

let rec lex_string = () =>
  Lexers([
    Lexer(
      quote,
      Any,
      lazy (Result(s => String(String.sub(s, 1, String.length(s) - 2)))),
    ),
    Lexer(Except([quote]), Any, lazy (lex_string())),
  ]);

let lexer = Lexer(quote, Any, lazy (lex_string()));