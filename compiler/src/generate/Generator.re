/**
 Converts an AST into code based on the output format
 */
open Kore;
open Target;

let generate = (target: Target.t, output: output_t, ast: program_t) =>
  switch (target) {
  | JavaScript(module_type) => JavaScript.generate(module_type, output, ast)
  | Knot => Grammar.Formatter.format(ast) |> output.print
  };
