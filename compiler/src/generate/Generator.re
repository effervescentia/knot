/**
 Converts an AST into code based on the output format
 */
open Kore;
open Target;

let generate = (print: string => unit, target: Target.t, ast: program_t) =>
  switch (target) {
  | JavaScript(module_type) => JavaScript.generate(print, module_type, ast)
  | Knot => Grammar.Formatter.format(ast) |> print
  };
