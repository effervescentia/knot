open Knot.Kore;
open AST;

let format: Fmt.t((string, Module.raw_declaration_t)) =
  (ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) => (name, expr) |> KConstant.Plugin.format(ppf)
    | Enumerated(variants) =>
      (name, variants) |> KEnumerated.Plugin.format(ppf)
    | Function(args, expr) =>
      (name, (args, expr)) |> KFunction.Plugin.format(ppf)
    | View(props, mixins, expr) =>
      (name, (props, mixins, expr)) |> KView.Plugin.format(ppf)
    };
