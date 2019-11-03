open Core;

/** keyword lexer matcher */
let (==>) = (s, t) => s ===> Keyword(t);

let matchers = [
  "import" ==> Import,
  "module" ==> Module,
  "from" ==> From,
  "const" ==> Const,
  "let" ==> Let,
  "func" ==> Func,
  "view" ==> View,
  "state" ==> State,
  "style" ==> Style,
  "else" ==> Else,
  "if" ==> If,
  "as" ==> As,
  "mut" ==> Mut,
  "get" ==> Get,
  "main" ==> Main,
];
