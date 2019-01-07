open Core;

let (===>) = Util.(===>);

let rec (==>) = (s, t) => s ===> Keyword(t);

let lexer =
  Lexers([
    "import" ==> Import,
    "from" ==> From,
    "const" ==> Const,
    "let" ==> Let,
    "func" ==> Func,
    "view" ==> View,
    "state" ==> State,
    "style" ==> Style,
    "else" ==> Else,
    "if" ==> If,
    "mut" ==> Mut,
    "get" ==> Get,
    "main" ==> Main,
  ]);