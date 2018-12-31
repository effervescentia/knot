open Core;
open Knot.Token;

let rec lexer =
  Lexers([
    Lexer(
      Numeric,
      Except([Numeric]),
      s => Result(Number(int_of_string(s))),
    ),
    Lexer(Numeric, Numeric, _ => lexer),
  ]);