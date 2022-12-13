open Kore;
open AST;

let format: Fmt.t((string, Module.raw_declaration_t)) =
  (ppf, (name, decl)) =>
    switch (decl) {
    | Constant(expr) => (name, expr) |> KConstant.format(ppf)
    | Enumerated(variants) => (name, variants) |> KEnumerated.format(ppf)
    | Function(args, expr) => (name, (args, expr)) |> KFunction.format(ppf)
    | View(props, mixins, expr) =>
      (name, (props, mixins, expr)) |> KView.format(ppf)
    };
