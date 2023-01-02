open Knot.Kore;
open AST;

let rec analyze_ksx:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.ksx_t) =>
  Result.ksx_t =
  (scope, analyze_expression, ksx) =>
    switch (ksx) {
    | Tag(_, view, styles, attrs, children) =>
      let tag_scope = Scope.create(scope.context, Node.get_range(view));
      tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

      let (name_type, tag_ast) =
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

      let id' = view |> Node.add_type(name_type);
      let styles' = styles |> List.map(analyze_expression(scope));
      let attrs' =
        attrs |> List.map(analyze_ksx_attribute(scope, analyze_expression));
      let children' =
        children |> List.map(analyze_ksx_child(scope, analyze_expression));

      let props =
        attrs'
        |> List.filter_map(
             fun
             | (((name, _), Some(expr)), meta) =>
               Some((name, (Node.get_type(expr), meta)))
             | (((name, _), None), meta) =>
               Some((
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
               )),
           );

      (
        (fst(view), name_type, props)
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

      (id', styles', attrs', children') |> tag_ast;

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
  (scope, analyze_expression, ((id, expr), _) as ksx_attr) => {
    let ksx_attr' = (id, expr |?> analyze_expression(scope));

    ksx_attr |> Node.map(_ => ksx_attr');
  }

and analyze_ksx_child:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.ksx_child_t
  ) =>
  Result.ksx_child_t =
  (scope, analyze_expression, ksx_child) => {
    let ksx_child' =
      switch (fst(ksx_child)) {
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

    ksx_child |> Node.map(_ => ksx_child');
  };

let analyze:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.ksx_t) =>
  (Result.ksx_t, Type.t) =
  (scope, analyze_expression, ksx) => {
    let ksx' = analyze_ksx(scope, analyze_expression, ksx);

    (ksx', Type.Valid(`Element));
  };
