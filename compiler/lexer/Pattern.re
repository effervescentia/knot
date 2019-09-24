open Core;

let _space = Char(' ');
let _tab = Char('\t');
let _whitespace = [_space, _tab, newline];

let rec _permiss = (lexer, chs) =>
  Lexers([lexer, Lexer(Either(chs), Any, _ => _permiss(lexer, chs))]);

let rec (==>) = (p, t) => {
  let next = _ =>
    if (String.length(p) == 1) {
      Result(t);
    } else {
      _permiss(String.sub(p, 1, String.length(p) - 1) ==> t, _whitespace);
    };

  Lexer(Char(p.[0]), Any, next);
};

let lexer =
  Lexers([
    "&&" ==> LogicalAnd,
    "||" ==> LogicalOr,
    "->" ==> Lambda,
    "==" ==> Equals,
    "!=" ==> NotEquals,
    "<=" ==> LessThanOrEqual,
    ">=" ==> GreaterThanOrEqual,
    "/>" ==> JSXSelfClose,
    "</" ==> JSXOpenEnd,
    "<>" ==> JSXStartFragment,
    "</>" ==> JSXEndFragment,
  ]);
