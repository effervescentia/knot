open Kore;
open AST;

let format: Fmt.t((string, Module.raw_declaration_t)) =
  (ppf, (name, decl)) => {
    let (&>) = (args, format) => (name, args) |> format(ppf);

    switch (decl) {
    | Constant(expr) => expr &> KConstant.format
    | Enumerated(variants) => variants &> KEnumerated.format
    | Function(args, expr) => (args, expr) &> KFunction.format
    | View(props, mixins, expr) => (props, mixins, expr) &> KView.format
    };
  };
