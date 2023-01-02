open Knot.Kore;
open AST;

let rec analyze_jsx:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.jsx_t) =>
  Result.jsx_t =
  (scope, analyze_expression, jsx) =>
    switch (jsx) {
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
        attrs |> List.map(analyze_jsx_attribute(scope, analyze_expression));
      let children' =
        children |> List.map(analyze_jsx_child(scope, analyze_expression));

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
        |> Validator.validate_jsx_render(!List.is_empty(children))
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
      |> List.map(analyze_jsx_child(scope, analyze_expression))
      |> Result.of_frag
    }

and analyze_jsx_attribute:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.jsx_attribute_t
  ) =>
  Result.jsx_attribute_t =
  (scope, analyze_expression, ((id, expr), _) as jsx_attr) => {
    let jsx_attr' = (id, expr |?> analyze_expression(scope));

    jsx_attr |> Node.map(_ => jsx_attr');
  }

and analyze_jsx_child:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    Raw.jsx_child_t
  ) =>
  Result.jsx_child_t =
  (scope, analyze_expression, jsx_child) => {
    let jsx_child' =
      switch (fst(jsx_child)) {
      | Text(text) => Result.of_text(text)

      | Node(jsx) =>
        jsx |> analyze_jsx(scope, analyze_expression) |> Result.of_node

      | InlineExpression(raw_expr) =>
        let expr = raw_expr |> analyze_expression(scope);
        let type_ = Node.get_type(expr);

        type_
        |> Validator.validate_jsx_primitive_expression
        |> Option.iter(
             expr |> Node.get_range |> Scope.report_type_err(scope),
           );

        Result.of_inline_expr(expr);
      };

    Node.untyped(jsx_child', Node.get_range(jsx_child));
  };

let analyze:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.jsx_t) =>
  (Result.jsx_t, Type.t) =
  (scope, analyze_expression, jsx) => {
    let jsx' = analyze_jsx(scope, analyze_expression, jsx);

    (jsx', Type.Valid(`Element));
  };
