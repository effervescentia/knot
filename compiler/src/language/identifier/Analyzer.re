open Knot.Kore;
open AST;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (_, scope, (name, _) as node) => {
    let resolve_error = err => {
      err |> Scope.report_type_err(scope, Node.get_range(node));
      Type.Invalid(NotInferrable);
    };

    let type_ =
      scope
      |> Scope.lookup(name)
      |> Option.fold(
           ~some=
             Stdlib.Result.fold(
               ~ok=Option.some,
               ~error=resolve_error % Option.some,
             ),
           ~none=None,
         )
      |!: (() => Type.NotFound(name) |> resolve_error);

    (name, type_);
  };
