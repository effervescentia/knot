module LazyStream = Opal.LazyStream;

let (%) = (f, g, x) => f(x) |> g;

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
  "mut",
  "get",
  "main",
  "true",
  "false",
];