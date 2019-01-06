open Core;

let (===>) = Util.(===>);

let rec (==>) = (s, t) => s ===> Keyword(t);

let lexer =
  Lexers([
    "import" ==> Import,
    "from" ==> From,
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