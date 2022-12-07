open Knot.Kore;
open AST;

let rec analyze_jsx:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.jsx_t) =>
  Result.jsx_t =
  (scope, analyze_expression, jsx) =>
    switch (jsx) {
    | Tag(id, styles, attrs, children) =>
      let tag_scope = Scope.create(scope.context, Node.get_range(id));
      tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

      let (id_type, tag_ast) =
        scope
        |> Scope.lookup(fst(id))
        |> Option.map(Tuple.with_snd2(Result.of_component))
        |?| (
          tag_scope
          |> Scope.lookup(fst(id))
          |> Option.map(Tuple.with_snd2(Result.of_tag))
        )
        |?: (Invalid(NotInferrable), Result.of_component);

      let id' = id |> Node.add_type(id_type);
      let styles' = styles |> List.map(analyze_expression(scope));
      let attrs' =
        attrs |> List.map(analyze_jsx_attribute(scope, analyze_expression));
      let children' =
        children |> List.map(analyze_jsx_child(scope, analyze_expression));

      let props =
        attrs'
        |> List.filter_map(
             fun
             | (((name, _), Some(expr)), range) =>
               Some((name, (Node.get_type(expr), range)))
             | (((name, _), None), range) =>
               Some((
                 name,
                 (
                   scope |> Scope.lookup(name) |?: Invalid(NotInferrable),
                   range,
                 ),
               )),
           );

      ((fst(id), id_type, props) |> Validator.validate_jsx_render)
      @ Validator.validate_style_binding(styles')
      |> List.iter(((err, err_range)) =>
           Scope.report_type_err(
             scope,
             err_range |?: Node.get_range(id),
             err,
           )
         );

      (id', styles', attrs', children') |> tag_ast;

    | Fragment(children) =>
      children
      |> List.map(analyze_jsx_child(scope, analyze_expression))
      |> Result.of_frag

    /* this should never be called, component delegation is determined at this point */
    | Component(_) => raise(SystemError)
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
