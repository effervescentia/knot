open Knot.Kore;
open AST;

let rec analyze_ksx:
  (
    Interface.Plugin.analyze_arg_t('ast, 'raw_expr, 'result_expr),
    Scope.t('ast),
    Interface.t('raw_expr, unit)
  ) =>
  Interface.t('result_expr, Type.t) =
  (analyze_expression, scope, ksx) =>
    switch (ksx) {
    | Tag(_, view, styles, attributes, children) =>
      let tag_scope = Scope.create(scope.context, Node.get_range(view));
      tag_scope |> Scope.inject_plugin_types(~prefix="", ElementTag);

      let (view_type, to_tag) =
        scope
        |> Scope.lookup(fst(view))
        |> Option.map(
             Stdlib.Result.map(Tuple.with_snd2(Interface.of_component_tag)),
           )
        |?| (
          tag_scope
          |> Scope.lookup(fst(view))
          |> Option.map(
               Stdlib.Result.map(Tuple.with_snd2(Interface.of_element_tag)),
             )
        )
        |> (
          fun
          | Some(Ok(x)) => x
          | Some(Error(err)) => {
              err |> Scope.report_type_err(scope, Node.get_range(view));
              (Invalid(NotInferrable), Interface.of_component_tag);
            }
          | None => (Invalid(NotInferrable), Interface.of_component_tag)
        );

      let view' = view |> Node.add_type(view_type);
      let styles' =
        styles |> List.map(Node.analyzer(analyze_expression(scope)) % fst);

      let attributes' =
        attributes
        |> List.map(analyze_ksx_attribute(analyze_expression, scope));
      let children' =
        children |> List.map(analyze_ksx_child(analyze_expression, scope));

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
      |> List.map(analyze_ksx_child(analyze_expression, scope))
      |> Interface.of_fragment
    }

and analyze_ksx_attribute:
  (
    Interface.Plugin.analyze_arg_t('ast, 'raw_expr, 'result_expr),
    Scope.t('ast),
    Interface.Attribute.node_t('raw_expr, unit)
  ) =>
  Interface.Attribute.node_t('result_expr, Type.t) =
  (analyze_expression, scope, ((id, expr), _) as attribute) => {
    let attribute' = (
      id,
      expr |?> Node.analyzer(analyze_expression(scope)) % fst,
    );

    attribute |> Node.map(_ => attribute');
  }

and analyze_ksx_child:
  (
    Interface.Plugin.analyze_arg_t('ast, 'raw_expr, 'result_expr),
    Scope.t('ast),
    Interface.Child.node_t('raw_expr, 'raw_ksx, unit)
  ) =>
  Interface.Child.node_t('result_expr, 'result_ksx, Type.t) =
  (analyze_expression, scope, child) => {
    let child' =
      switch (fst(child)) {
      | Text(text) => Interface.Child.of_text(text)

      | Node(ksx) =>
        ksx
        |> analyze_ksx(analyze_expression, scope)
        |> Interface.Child.of_node

      | InlineExpression(expression) =>
        let (expression', expression_type) =
          expression |> Node.analyzer(analyze_expression(scope));

        expression_type
        |> Validator.validate_ksx_primitive_expression
        |> Option.iter(
             Scope.report_type_err(scope, Node.get_range(expression')),
           );

        Interface.Child.of_inline(expression');
      };

    child |> Node.map(_ => child');
  };

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, (ksx, _)) => {
    let ksx' = analyze_ksx(analyze_expression, scope, ksx);

    (ksx', Type.Valid(Element));
  };
