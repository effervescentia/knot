open Kore;

let generate = (target: target_t, ast: AST.program_t) =>
  switch (target) {
  | JavaScript(module_type) => ""
  | Knot => Grammar.Formatter.format(ast)
  };
