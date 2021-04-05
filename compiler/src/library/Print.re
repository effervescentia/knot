/**
 Repackaging of the Printf module with additional functionality.
 */
open Infix;

include Printf;

module ANSI = ANSITerminal;

let color = ref(false);

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

let ansi_sprintf = (x, s) => color^ ? ANSI.sprintf(x, "%s", s) : s;
let ansi_code_sprintf = (x, s) =>
  color^ ? fmt("\027[;%dm%s\027[0m", x, s) : s;

let bold = ansi_sprintf([ANSI.Bold]);
let red = ansi_sprintf([ANSI.red]);
let green = ansi_sprintf([ANSI.green]);
let cyan = ansi_sprintf([ANSI.cyan]);
let yellow = ansi_sprintf([ANSI.yellow]);
let grey = ansi_code_sprintf(90);

let good = green % bold;
let bad = red % bold;
let warn = yellow % bold;
