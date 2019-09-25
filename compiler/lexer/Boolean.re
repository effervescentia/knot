open Core;

let lexer =
  Lexers(["true" ===> Boolean(true), "false" ===> Boolean(false)]);
