open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Expression.ViewKind.t, Raw.expression_t, Raw.expression_t),
    Range.t
  ) =>
  (Expression.ViewKind.t, Result.expression_t, Result.expression_t) =
  (scope, analyze_expression, (kind, view, style), range) => {
    let lhs_range = Node.get_range(view);
    let tag_scope = Scope.create(scope.context, lhs_range);
    tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

    let (kind', view', view_type) =
      switch (fst(view)) {
      | Identifier(id) =>
        let (view_type, kind') =
          scope
          |> Scope.lookup(id)
          |> Option.map(
               Stdlib.Result.map(
                 Tuple.with_snd2(Expression.ViewKind.Component),
               ),
             )
          |?| (
            tag_scope
            |> Scope.lookup(id)
            |> Option.map(
                 Stdlib.Result.map(
                   Tuple.with_snd2(Expression.ViewKind.Element),
                 ),
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
          |?: (Invalid(NotInferrable), Expression.ViewKind.Component);

        (
          kind',
          Node.typed(Expression.Identifier(id), view_type, lhs_range),
          view_type,
        );

      | _ =>
        analyze_expression(scope, view)
        |> Tuple.split3(
             _ => Expression.ViewKind.Component,
             Fun.id,
             Node.get_type,
           )
      };

    let style' = analyze_expression(scope, style);
    let style_type = Node.get_type(style');

    (view_type, style_type)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (kind', view', style');
  };
