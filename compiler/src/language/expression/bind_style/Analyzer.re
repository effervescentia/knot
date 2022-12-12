open Knot.Kore;
open AST;

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    (Raw.bind_style_target_t, Raw.expression_t),
    Range.t
  ) =>
  (Result.bind_style_target_t, Result.expression_t) =
  (scope, analyze_expression, (view, style), range) => {
    let (view_expr, view_type) =
      switch (view) {
      | BuiltIn(expr)
      | Local(expr) =>
        let tag_scope = Scope.create(scope.context, Node.get_range(expr));
        tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

        switch (expr) {
        | (Identifier(id), _) =>
          let (type_, tag_ast) =
            scope
            |> Scope.lookup(id)
            |> Option.map(
                 Stdlib.Result.map(Tuple.with_snd2(Result.of_local)),
               )
            |?| (
              tag_scope
              |> Scope.lookup(id)
              |> Option.map(
                   Stdlib.Result.map(Tuple.with_snd2(Result.of_builtin)),
                 )
            )
            |> (
              fun
              | Some(Ok(x)) => x
              | Some(Error(err)) => {
                  err |> Scope.report_type_err(scope, Node.get_range(expr));
                  (Invalid(NotInferrable), Result.of_local);
                }
              | None => (Invalid(NotInferrable), Result.of_local)
            );

          (
            Node.typed(
              Expression.Identifier(id),
              type_,
              Node.get_range(expr),
            )
            |> tag_ast,
            type_,
          );
        | _ =>
          let expr' = analyze_expression(scope, expr);

          (Result.of_local(expr'), Node.get_type(expr'));
        };
      };

    let style' = analyze_expression(scope, style);
    let style_type = Node.get_type(style');

    (view_type, style_type)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    (view_expr, style');
  };
