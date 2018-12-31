open Knot.Token;
open Core;

let rec (==>) = (s, t) => {
  let next = _ =>
    if (String.length(s) == 1) {
      Result(Keyword(t));
    } else {
      String.sub(s, 1, String.length(s) - 1) ==> t;
    };

  Lexer(Char(s.[0]), Any, next);
};

let lexer =
  Lexers([
    "import" ==> Import,
    "const" ==> Const,
    "let" ==> Let,
    "state" ==> State,
    "view" ==> View,
    "func" ==> Func,
    "else" ==> Else,
    "if" ==> If,
    "mut" ==> Mut,
    "get" ==> Get,
    "main" ==> Main,
  ]);