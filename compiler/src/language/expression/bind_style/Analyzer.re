open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Expression.view_kind_t, Raw.expression_t, Raw.expression_t),
    Range.t
  ) =>
  (Expression.view_kind_t, Result.expression_t, Result.expression_t) =
  (scope, analyze_expression, (kind, lhs, rhs), range) => {
    let lhs_range = Node.get_range(lhs);
    let tag_scope = Scope.create(scope.context, lhs_range);
    tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

    let (kind', lhs', lhs_type) =
      switch (fst(lhs)) {
      | Identifier(id) =>
        let (lhs_type, kind') =
          scope
          |> Scope.lookup(id)
          |> Option.map(
               Stdlib.Result.map(Tuple.with_snd2(Expression.Component)),
             )
          |?| (
            tag_scope
            |> Scope.lookup(id)
            |> Option.map(
                 Stdlib.Result.map(Tuple.with_snd2(Expression.Element)),
               )
          )
          |> (
            fun
            | Some(Ok(ok)) => Some(ok)
            | Some(Error(err)) => {
                err |> Scope.report_type_err(scope, lhs_range);
                None;
              }
            | None => None
          )
          |?: (Invalid(NotInferrable), Expression.Component);

        (
          kind',
          Node.typed(Expression.Identifier(id), lhs_type, lhs_range),
          lhs_type,
        );

      | _ =>
        analyze_expression(scope, lhs)
        |> Tuple.split3(_ => Expression.Component, Fun.id, Node.get_type)
      };

    let rhs' = analyze_expression(scope, rhs);
    let rhs_type = Node.get_type(rhs');

    (lhs_type, rhs_type)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (kind', lhs', rhs');
  };
