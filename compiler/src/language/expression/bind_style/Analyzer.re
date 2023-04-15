open Knot.Kore;
open AST;

module ViewKind = KSX.Interface.ViewKind;

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (
    (analyze_expression, (from_id, to_id)),
    scope,
    ((_, view, style), _) as node,
  ) => {
    let range = Node.get_range(node);
    let lhs_range = Node.get_range(view);
    let tag_scope = Scope.create(scope.context, lhs_range);
    tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

    let (kind, view', view_type) =
      switch (from_id(fst(view))) {
      | Some(id) =>
        let (view_type, kind) =
          scope
          |> Scope.lookup(id)
          |> Option.map(
               Stdlib.Result.map(Tuple.with_snd2(ViewKind.Component)),
             )
          |?| (
            tag_scope
            |> Scope.lookup(id)
            |> Option.map(
                 Stdlib.Result.map(Tuple.with_snd2(ViewKind.Element)),
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
          |?: (Invalid(NotInferrable), ViewKind.Component);

        (kind, Node.typed(to_id(id), view_type, lhs_range), view_type);

      | None =>
        view
        |> Node.analyzer(analyze_expression(scope))
        |> Tuple.split3(_ => ViewKind.Component, fst, snd)
      };

    let (style', style_type) =
      style |> Node.analyzer(analyze_expression(scope));

    (view_type, style_type)
    |> Validator.validate
    |> Option.iter(Scope.report_type_err(scope, range));

    ((kind, view', style'), view_type);
  };
