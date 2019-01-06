module LazyStream = Opal.LazyStream;

let (%) = (f, g, x) => f(x) |> g;

let reserved = [
  "import",
  "from",
  "const",
  "let",
  "state",
  "view",
  "func",
  "else",
  "if",
  "mut",
  "get",
  "main",
  "true",
  "false",
];