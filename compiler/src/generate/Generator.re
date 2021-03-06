/**
 * Converts an AST into code based on the output format
 */
open Kore;
open Target;

let generate = (target: Target.t, ast: AST.program_t) =>
  switch (target) {
  | JavaScript(module_type) => ""
  | Knot => Grammar.Formatter.format(ast)
  };
