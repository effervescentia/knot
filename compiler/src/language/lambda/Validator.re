open Knot.Kore;
open AST;

let rec validate_default_arguments =
        (
          ~require_default=false,
          scope: Scope.t,
          args: list(Result.argument_t),
        ) =>
  switch (args, require_default) {
  | ([], _) => ()

  | ([({name: (name, _), default: None, _}, _) as arg, ...xs], true) =>
    Type.DefaultArgumentMissing(name)
    |> Scope.report_type_err(scope, Node.get_range(arg));

    validate_default_arguments(~require_default, scope, xs);

  | ([({default: Some(_), _}, _), ...xs], _) =>
    validate_default_arguments(~require_default=true, scope, xs)

  | ([_, ...xs], _) =>
    validate_default_arguments(~require_default, scope, xs)
  };
