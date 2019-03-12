include Exception;

module LazyStream = Opal.LazyStream;
module Log = Log;
module Emoji = Emoji;

let (%) = (f, g, x) => f(x) |> g;

let (|-) = (f, g, x) =>
  f(x)
  |> (
    y => {
      g(x);
      y;
    }
  );

let reserved = [
  "import",
  "from",
  "const",
  "let",
  "func",
  "view",
  "state",
  "style",
  "else",
  "if",
  "as",
  "mut",
  "get",
  "main",
  "true",
  "false",
];

let main_alias = "[main]";
