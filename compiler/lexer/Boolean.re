open Core;
open Util;

let lexer =
  Lexers(["true" ===> Boolean(true), "false" ===> Boolean(false)]);
