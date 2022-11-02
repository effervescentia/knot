/**
 Converts an AST into code based on the output format
 */
open Kore;

let pp = (target: Target.t, resolve: resolve_t): Fmt.t(AST.Result.program_t) =>
  (ppf, program) =>
    switch (target) {
    | JavaScript(module_type) =>
      program
      |> JavaScript_Generator.generate(resolve)
      |> JavaScript_Formatter.format(module_type, ppf)

    | Knot => program |> Language.Formatter.format(ppf)
    };
