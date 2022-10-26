open Knot.Kore;

let validate_jsx_render:
  ((string, Type.t, list((string, AST.untyped_t(Type.t))))) =>
  list((Type.error_t, option(Range.t))) =
  fun
  /* assume this have been reported already and ignore */
  | (_, Invalid(_), _) => []

  | (id, Valid(`View(attrs, _)), actual_attrs) => {
      let keys =
        attrs
        @ (actual_attrs |> List.map(Tuple.map_snd2(fst)))
        |> List.map(fst)
        |> List.uniq_by((==));

      let (invalid, missing) =
        keys
        |> List.fold_left(
             ((invalid, missing) as acc, key) => {
               let expected = attrs |> List.assoc_opt(key);
               let actual = actual_attrs |> List.assoc_opt(key);

               switch (expected, actual) {
               | (Some(expected'), Some((actual_value, _) as actual')) =>
                 switch (expected', actual_value) {
                 | (Type.Invalid(_), _)
                 | (_, Type.Invalid(_)) => acc
                 | (Type.Valid(_), Type.Valid(_))
                     when expected' == actual_value => acc
                 | (Type.Valid(_), Type.Valid(_)) => (
                     invalid
                     @ [
                       (
                         Type.InvalidJSXAttribute(
                           key,
                           expected',
                           actual_value,
                         ),
                         Some(Node.get_range(actual')),
                       ),
                     ],
                     missing,
                   )
                 }

               | (Some(expected'), None) => (
                   invalid,
                   missing @ [(key, expected')],
                 )

               | (None, Some(actual')) => (
                   invalid
                   @ [
                     (
                       Type.UnexpectedJSXAttribute(key, fst(actual')),
                       Some(Node.get_range(actual')),
                     ),
                   ],
                   missing,
                 )

               | (None, None) => acc
               };
             },
             ([], []),
           );

      if (!List.is_empty(invalid)) {
        invalid;
      } else if (!List.is_empty(missing)) {
        [(Type.MissingJSXAttributes(id, missing), None)];
      } else {
        [];
      };
    }

  | (id, expr_type, attrs) => [
      (
        InvalidJSXTag(id, expr_type, attrs |> List.map(Tuple.map_snd2(fst))),
        None,
      ),
    ];

let validate_jsx_class_expression: Type.t => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Boolean) => None

  | type_ => Some(InvalidJSXClassExpression(type_));

let validate_jsx_primitive_expression: Type.t => option(Type.error_t) =
  fun
  /* assume this has been reported already and ignore */
  | Invalid(_) => None

  | Valid(`Nil | `Boolean | `Integer | `Float | `String | `Element) => None

  | type_ => Some(InvalidJSXPrimitiveExpression(type_));

let rec analyze_jsx:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.jsx_t
  ) =>
  AST.jsx_t =
  (scope, analyze_expression, jsx) => (
    switch (jsx) {
    | Tag(id, attrs, children) =>
      let attrs' =
        attrs |> List.map(analyze_jsx_attribute(scope, analyze_expression));
      let children' =
        children |> List.map(analyze_jsx_child(scope, analyze_expression));
      let is_tag_capitalized = id |> fst |> String.is_capitalized;

      if (is_tag_capitalized) {
        let id_type =
          scope |> Scope.lookup(fst(id)) |?: Type.Invalid(NotInferrable);
        let props =
          attrs'
          |> List.filter_map(
               fun
               | (AST.Property((name, _), Some(expr)), range) =>
                 Some((name, (Node.get_type(expr), range)))
               | (_, range) => None,
             );

        (fst(id), id_type, props)
        |> validate_jsx_render
        |> List.iter(((err, err_range)) =>
             Scope.report_type_err(
               scope,
               err_range |?: Node.get_range(id),
               err,
             )
           );

        (id |> Node.add_type(id_type), attrs', children') |> AST.of_component;
      } else {
        (id, attrs', children') |> AST.of_tag;
      };

    | Fragment(children) =>
      children
      |> List.map(analyze_jsx_child(scope, analyze_expression))
      |> AST.of_frag

    | Component(_) => raise(SystemError)
    }: AST.jsx_t
    /* this should never be called, component delegation is determined at this point */
  )

and analyze_jsx_attribute:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.jsx_attribute_t
  ) =>
  AST.jsx_attribute_t =
  (scope, analyze_expression, jsx_attr) => {
    let jsx_attr' =
      switch (fst(jsx_attr)) {
      | ID(id) => AST.of_jsx_id(id)

      | Class(id, raw_expr) =>
        let expr_opt = raw_expr |?> analyze_expression(scope);

        expr_opt
        |> Option.iter(expr => {
             let type_ = Node.get_type(expr);

             type_
             |> validate_jsx_class_expression
             |> Option.iter(
                  expr |> Node.get_range |> Scope.report_type_err(scope),
                );
           });

        (id, expr_opt) |> AST.of_jsx_class;

      | Property(id, expr) =>
        (id, expr |?> analyze_expression(scope)) |> AST.of_prop
      };

    Node.untyped(jsx_attr', Node.get_range(jsx_attr));
  }

and analyze_jsx_child:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.jsx_child_t
  ) =>
  AST.jsx_child_t =
  (scope, analyze_expression, jsx_child) => {
    let jsx_child' =
      switch (fst(jsx_child)) {
      | Text(text) => AST.of_text(text)

      | Node(jsx) =>
        jsx |> analyze_jsx(scope, analyze_expression) |> AST.of_node

      | InlineExpression(raw_expr) =>
        let expr = raw_expr |> analyze_expression(scope);
        let type_ = Node.get_type(expr);

        type_
        |> validate_jsx_primitive_expression
        |> Option.iter(
             expr |> Node.get_range |> Scope.report_type_err(scope),
           );

        AST.of_inline_expr(expr);
      };

    Node.untyped(jsx_child', Node.get_range(jsx_child));
  };

let rec analyze_root:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.expression_t,
    AST.Raw.jsx_t
  ) =>
  (AST.jsx_t, Type.t) =
  (scope, analyze_expression, jsx) => {
    let jsx' = analyze_jsx(scope, analyze_expression, jsx);

    (jsx', Type.Valid(`Element));
  };
