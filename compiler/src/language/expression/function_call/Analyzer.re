open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.expression_t, list(Raw.expression_t)),
    Range.t
  ) =>
  (Result.expression_t, list(Result.expression_t), Type.t) =
  (scope, analyze_expression, (function_, arguments), range) => {
    let function_' = analyze_expression(scope, function_);
    let arguments' = arguments |> List.map(analyze_expression(scope));
    let function_type = Node.get_type(function_');
    let argument_types = arguments' |> List.map(Node.get_type);

    (function_type, argument_types)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      function_',
      arguments',
      switch (function_type) {
      | Valid(Function(_, result)) => result
      | _ => Invalid(NotInferrable)
      },
    );
  };
