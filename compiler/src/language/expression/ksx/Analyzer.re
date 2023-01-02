open Knot.Kore;
open AST;

let rec analyze_ksx:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.ksx_t) =>
  Result.ksx_t =
  (scope, analyze_expression, ksx) =>
    switch (ksx) {
    | Tag(_, view, styles, attributes, children) =>
      let tag_scope = Scope.create(scope.context, Node.get_range(view));
      tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

      let (view_type, to_tag) =
        scope
        |> Scope.lookup(fst(view))
        |> Option.map(
             Stdlib.Result.map(Tuple.with_snd2(Result.of_component_tag)),
           )
        |?| (
          tag_scope
          |> Scope.lookup(fst(view))
          |> Option.map(
               Stdlib.Result.map(Tuple.with_snd2(Result.of_element_tag)),
             )
        )
        |> (
          fun
          | Some(Ok(x)) => x
          | Some(Error(err)) => {
              err |> Scope.report_type_err(scope, Node.get_range(view));
              (Invalid(NotInferrable), Result.of_component_tag);
            }
          | None => (Invalid(NotInferrable), Result.of_component_tag)
        );

      let view' = view |> Node.add_type(view_type);
      let styles' = styles |> List.map(analyze_expression(scope));
      let attributes' =
        attributes
        |> List.map(analyze_ksx_attribute(scope, analyze_expression));
      let children' =
        children |> List.map(analyze_ksx_child(scope, analyze_expression));

      let punned_attributes =
        attributes'
        |> List.map(
             fun
             | (((name, _), Some(expr)), meta) => (
                 name,
                 (Node.get_type(expr), meta),
               )
             | (((name, _), None), meta) => (
                 name,
                 (
                   scope
                   |> Scope.lookup(name)
                   |> (
                     fun
                     | Some(Ok(x)) => x
                     | Some(Error(err)) => {
                         err |> Scope.report_type_err(scope, snd(meta));
                         Invalid(NotInferrable);
                       }
                     | None => Invalid(NotInferrable)
                   ),
                   meta,
                 ),
               ),
           );

      (
        (fst(view), view_type, punned_attributes)
        |> Validator.validate_ksx_render(!List.is_empty(children))
      )
      @ Validator.validate_style_binding(styles')
      |> List.iter(((err, err_range)) =>
           Scope.report_type_err(
             scope,
             err_range |?: Node.get_range(view),
             err,
           )
         );

      (view', styles', attributes', children') |> to_tag;

    | Fragment(children) =>
      children
      |> List.map(analyze_ksx_child(scope, analyze_expression))
      |> Result.of_frag
    }

and analyze_ksx_attribute:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.ksx_attribute_t
  ) =>
  Result.ksx_attribute_t =
  (scope, analyze_expression, ((id, expr), _) as attribute) => {
    let attribute' = (id, expr |?> analyze_expression(scope));

    attribute |> Node.map(_ => attribute');
  }

and analyze_ksx_child:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.ksx_child_t
  ) =>
  Result.ksx_child_t =
  (scope, analyze_expression, child) => {
    let child' =
      switch (fst(child)) {
      | Text(text) => Result.of_text(text)

      | Node(ksx) =>
        ksx |> analyze_ksx(scope, analyze_expression) |> Result.of_node

      | InlineExpression(raw_expr) =>
        let expr = raw_expr |> analyze_expression(scope);
        let type_ = Node.get_type(expr);

        type_
        |> Validator.validate_ksx_primitive_expression
        |> Option.iter(
             expr |> Node.get_range |> Scope.report_type_err(scope),
           );

        Result.of_inline_expr(expr);
      };

    child |> Node.map(_ => child');
  };

let analyze:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.ksx_t) =>
  (Result.ksx_t, Type.t) =
  (scope, analyze_expression, ksx) => {
    let ksx' = analyze_ksx(scope, analyze_expression, ksx);

    (ksx', Type.Valid(Element));
  };
