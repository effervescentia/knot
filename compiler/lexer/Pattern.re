open Core;
open Knot.Token;

let whitespace = [Char(' '), Char('\t'), newline];

let rec permiss = (lexer, chs) =>
  Lexers([lexer, Lexer(Either(chs), Any, _ => permiss(lexer, chs))]);

let rec (==>) = (p, t) => {
  let next = _ =>
    if (String.length(p) == 1) {
      Result(t);
    } else {
      permiss(String.sub(p, 1, String.length(p) - 1) ==> t, whitespace);
    };

  Lexer(Char(p.[0]), Any, next);
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