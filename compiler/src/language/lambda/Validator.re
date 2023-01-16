open Knot.Kore;
open AST;

let rec validate_default_arguments =
        (
          ~require_default=false,
          scope: Scope.t('ast),
          args: list(Interface.Parameter.node_t('expr, Type.t)),
        ) =>
  switch (args, require_default) {
  | ([], _) => ()

  | ([(((name, _), _, None), _) as arg, ...xs], true) =>
    Type.DefaultArgumentMissing(name)
    |> Scope.report_type_err(scope, Node.get_range(arg));

    validate_default_arguments(~require_default, scope, xs);

  | ([((_, _, Some(_)), _), ...xs], _) =>
    validate_default_arguments(~require_default=true, scope, xs)

  | ([_, ...xs], _) =>
    validate_default_arguments(~require_default, scope, xs)
  };
