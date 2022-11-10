open Knot.Kore;
open AST;

let validate_dot_access: (string, Type.t) => option(Type.error_t) =
  prop =>
    fun
    /* assume this has been reported already and ignore */
    | Invalid(_) => None

    | Valid(`Struct(props))
        when props |> List.exists(((name, _)) => name == prop) =>
      None

    | Valid(`Module(entries))
        when entries |> List.exists(((name, _)) => name == prop) =>
      None

    | type_ => Some(InvalidDotAccess(type_, prop));

let analyze_dot_access:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.expression_t, Node.t(string, unit)),
    Range.t
  ) =>
  (Result.expression_t, Type.t) =
  (scope, analyze_expression, (expr, prop), range) => {
    let prop_name = fst(prop);
    let expr' = analyze_expression(scope, expr);
    let type_ = Node.get_type(expr');

    type_
    |> validate_dot_access(prop_name)
    |> Option.iter(Scope.report_type_err(scope, range));

    (
      expr',
      (
        switch (type_) {
        | Valid(`Struct(props)) => props |> List.assoc_opt(prop_name)

        | Valid(`Module(entries)) =>
          entries
          |> List.find_map(
               fun
               | (name, Type.Container.Value(t)) when name == prop_name =>
                 Some(t)
               | _ => None,
             )

        | _ => None
        }
      )
      |?: Invalid(NotInferrable),
    );
  };
