open Core;
open Knot.Token;

let whitespace = [' ', '\t', '\n'];

let permiss = (lexer, chs) =>
  Lexers([
    lexer,
    ...List.map(ch => Lexer(Char(ch), Any, lazy lexer), chs),
  ]);

let rec (==>) = (p, t) => {
  let next = () =>
    if (String.length(p) == 1) {
      Result(_ => t);
    } else {
      permiss(String.sub(p, 1, String.length(p) - 1) ==> t, whitespace);
    };

  Lexer(Char(p.[0]), Any, lazy (next()));
};

let lexer =
  Lexers([
    "&&" ==> LogicalAnd,
    "||" ==> LogicalOr,
    "->" ==> Lambda,
    "==" ==> Equals,
    "<=" ==> LessThanOrEqual,
    ">=" ==> GreaterThanOrEqual,
    "/>" ==> JSXSelfClose,
    "</" ==> JSXOpenEnd,
  ]);