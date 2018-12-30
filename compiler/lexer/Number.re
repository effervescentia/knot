open Core;
open Knot.Token;

let rec lex_number = () =>
  Lexers([
    Lexer(
      Numeric,
      Except([Numeric]),
      lazy (Result(s => Number(int_of_string(s)))),
    ),
    Lexer(Numeric, Numeric, lazy (lex_number())),
  ]);

let lexer = lex_number();