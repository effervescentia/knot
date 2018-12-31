open Core;
open Knot.Token;

let whitespace = [Char(' '), Char('\t'), Char('\n')];

let rec permiss = (lexer, chs) =>
  Lexers([lexer, Lexer(Either(chs), Any, _ => permiss(lexer, chs))]);

let rec (==>) = (p, t) => {
  let next = _ =>
    if (String.length(p) == 1) {
      /* print_endline("returning result of pattern " ++ p); */
      Result(t);
    } else {
      /* print_endline("returning permissive matchers for " ++ p); */
      permiss(
        String.sub(p, 1, String.length(p) - 1) ==> t,
        whitespace,
      );
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