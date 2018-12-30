open Knot.Token;
open Core;

module FileStream = Knot.FileStream;

let rec (==>) = (s, t) => {
  let next = () =>
    if (String.length(s) == 1) {
      Result(_ => Keyword(t));
    } else {
      String.sub(s, 1, String.length(s) - 1) ==> t;
    };

  Lexer(Char(s.[0]), Any, lazy (next()));
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