open Knot.Kore;

let analyze_dot_access:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    (AST.Raw.expression_t, Node.t(string, unit)),
    Range.t
  ) =>
  (AST.expression_t, Type.t) =
  (scope, analyze_expression, (expr, prop), range) => {
    let prop_name = fst(prop);
    let expr' = analyze_expression(scope, expr);
    let type_ = Node.get_type(expr');

    type_
    |> Analyze.Typing.check_dot_access(prop_name)
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
