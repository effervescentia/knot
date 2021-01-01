include Printf;

let fmt = sprintf;

let rec many = (~separator="", print: 'a => string) =>
  fun
  | [] => ""
  | [x] => print(x)
  | [x, ...xs] => print(x) ++ separator ++ many(~separator, print, xs);
