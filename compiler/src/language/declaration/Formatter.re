open Knot.Kore;
open AST;

let pp_declaration: Fmt.t((string, Module.raw_declaration_t)) =
  (ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) => (name, expr) |> KConstant.Plugin.pp(ppf)
    | Enumerated(variants) => (name, variants) |> KEnumerated.Plugin.pp(ppf)
    | Function(args, expr) =>
      (name, (args, expr)) |> KFunction.Plugin.pp(ppf)
    | View(props, mixins, expr) =>
      (name, (props, mixins, expr)) |> KView.Plugin.pp(ppf)
    };
