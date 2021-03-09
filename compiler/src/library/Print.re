/**
 Repackaging of the Printf module with additional functionality.
 */
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
