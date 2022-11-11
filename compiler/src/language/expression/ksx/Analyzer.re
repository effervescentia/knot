open Knot.Kore;
open AST;

let validate_jsx_render:
  ((string, Type.t, list((string, Result.untyped_t(Type.t))))) =>
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
                 Type.(
                   switch (expected', actual_value) {
                   | (Invalid(_), _)
                   | (_, Invalid(_)) => acc
                   | (Valid(_), Valid(_)) when expected' == actual_value => acc
                   | (Valid(_), Valid(_)) => (
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
                 )

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
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.jsx_t) =>
  Result.jsx_t =
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
               | (((name, _), Some(expr)), range) =>
                 Some((name, (Node.get_type(expr), range)))
               | _ => None,
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

        (id |> Node.add_type(id_type), attrs', children')
        |> Result.of_component;
      } else {
        (id, attrs', children') |> Result.of_tag;
      };

    | Fragment(children) =>
      children
      |> List.map(analyze_jsx_child(scope, analyze_expression))
      |> Result.of_frag

    | Component(_) => raise(SystemError)
    }: Result.jsx_t
    /* this should never be called, component delegation is determined at this point */
  )

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
        |> validate_jsx_primitive_expression
        |> Option.iter(
             expr |> Node.get_range |> Scope.report_type_err(scope),
           );

        Result.of_inline_expr(expr);
      };

    Node.untyped(jsx_child', Node.get_range(jsx_child));
  };

let analyze_root:
  (Scope.t, (Scope.t, Raw.expression_t) => Result.expression_t, Raw.jsx_t) =>
  (Result.jsx_t, Type.t) =
  (scope, analyze_expression, jsx) => {
    let jsx' = analyze_jsx(scope, analyze_expression, jsx);

    (jsx', Type.Valid(`Element));
  };
