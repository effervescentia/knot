open Kore;

let fold = (~constant, ~enumerated, ~function_, ~view) =>
  AST.Declaration.(
    fun
    | Constant(expr) => expr |> constant
    | Enumerated(variants) => variants |> enumerated
    | Function(args, expr) => (args, expr) |> function_
    | View(props, mixins, expr) => (props, mixins, expr) |> view
  );
