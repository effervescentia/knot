/**
 Repackaging of the Printf module with additional functionality.
 */
open Infix;

include Printf;

module ANSI = ANSITerminal;

let fmt = sprintf;

let opt = f =>
  fun
  | Some(x) => f(x)
  | None => "";

let rec many = (~separator="", print: 'a => string) =>
  fun
  | [] => ""
  | [x] => print(x)
  | [x, ...xs] => print(x) ++ separator ++ many(~separator, print, xs);

let bold = ANSI.sprintf([ANSI.Bold], "%s");
let red = ANSI.sprintf([ANSI.red], "%s");
let green = ANSI.sprintf([ANSI.green], "%s");
let cyan = ANSI.sprintf([ANSI.cyan], "%s");
let yellow = ANSI.sprintf([ANSI.yellow], "%s");

let good = green % bold;
let bad = red % bold;
let warn = yellow % bold;
