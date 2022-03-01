/**
 Converts an AST into code based on the output format
 */
open Kore;

let generate =
    (
      target: Target.t,
      resolve: resolve_t,
      program: ASTV2.program_t,
      ppf: Format.formatter,
    ) =>
  switch (target) {
  | JavaScript(module_type) =>
    program
    |> JavaScript_Generator.generate(resolve)
    |> JavaScript_Formatter.format(module_type, ppf)

  | Knot => program |> Grammar.Formatter.format(ppf)
  };
