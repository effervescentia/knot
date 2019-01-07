open Core;

let (===>) = Util.(===>);

let lexer =
  Lexers(["true" ===> Boolean(true), "false" ===> Boolean(false)]);