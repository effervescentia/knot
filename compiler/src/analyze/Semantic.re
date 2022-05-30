open Kore;

type reporter_t = (Range.t, T.invalid_t) => unit;

let rec analyze_jsx = (scope: S.t, raw_jsx: AR.jsx_t): A.jsx_t =>
  switch (raw_jsx) {
  | Tag(id, attrs, children) =>
    (
      id,
      attrs |> List.map(analyze_jsx_attribute(scope)),
      children |> List.map(analyze_jsx_child(scope)),
    )
    |> A.of_tag

  | Fragment(children) =>
    children |> List.map(analyze_jsx_child(scope)) |> A.of_frag
  }

and analyze_jsx_attribute =
    (scope: S.t, (raw_jsx_attr, range): AR.jsx_attribute_t)
    : A.jsx_attribute_t => {
  let jsx_attr =
    switch (raw_jsx_attr) {
    | ID(id) => A.of_jsx_id(id)

    | Class(id, raw_expr) =>
      let expr_opt = raw_expr |?> analyze_expression(scope);

      expr_opt
      |> Option.iter(expr => {
           let type_ = N.get_type(expr);

           type_
           |> Typing.check_jsx_class_expression
           |> Option.iter(expr |> N.get_range |> S.report_type_err(scope));
         });

      (id, expr_opt) |> A.of_jsx_class;

    | Property(id, expr) =>
      (id, expr |?> analyze_expression(scope)) |> A.of_prop
    };

  NR.create(jsx_attr, range);
}

and analyze_jsx_child =
    (scope: S.t, (raw_jsx_child, range): AR.jsx_child_t): A.jsx_child_t => {
  let jsx_child =
    switch (raw_jsx_child) {
    | Text(text) => A.of_text(text)

    | Node(jsx) => jsx |> analyze_jsx(scope) |> A.of_node

    | InlineExpression(raw_expr) =>
      let expr = raw_expr |> analyze_expression(scope);
      let type_ = N.get_type(expr);

      type_
      |> Typing.check_jsx_primitive_expression
      |> Option.iter(expr |> N.get_range |> S.report_type_err(scope));

      A.of_inline_expr(expr);
    };

  NR.create(jsx_child, range);
}

and analyze_expression =
    (scope: S.t, (raw_expr, raw_type, range): AR.expression_t)
    : A.expression_t => {
  let (expr, type_) =
    switch (raw_expr) {
    /* should always be able to rely on the parser to have typed these accurately */
    | Primitive(prim) => (A.of_prim(prim), T.of_raw(raw_type))

    /* use the type of the inner analyzed expression */
    | Group(expr) =>
      let analyzed = analyze_expression(scope, expr);

      (A.of_group(analyzed), N.get_type(analyzed));

    /* use the type of the last analyzed statement or nil when empty */
    | Closure(stmts) =>
      let analyzed = stmts |> List.map(analyze_statement(scope));
      let type_ = analyzed |> List.last |?> N.get_type |?: T.Valid(`Nil);

      (A.of_closure(analyzed), type_);

    | JSX(jsx) => (jsx |> analyze_jsx(scope) |> A.of_jsx, T.Valid(`Element))

    | Identifier(id) =>
      let type_opt = scope |> S.lookup(id);

      let type_ =
        type_opt
        |!: (
          () => {
            let err = T.NotFound(id);

            err |> S.report_type_err(scope, range);

            T.Invalid(NotInferrable);
          }
        );

      (A.of_id(id), type_);

    | UnaryOp(op, expr) =>
      let analyzed = analyze_expression(scope, expr);
      let type_ = N.get_type(analyzed);

      type_
      |> Typing.check_unary_operation(op)
      |> Option.iter(S.report_type_err(scope, range));

      (
        (op, analyzed) |> A.of_unary_op,
        switch (op) {
        | Negative
        | Positive =>
          switch (type_) {
          | Valid(`Integer | `Float)
          /* forward invalid types */
          | Invalid(_) => type_

          | _ => Invalid(NotInferrable)
          }

        | Not => Valid(`Boolean)
        },
      );

    | BinaryOp(op, lhs, rhs) =>
      let analyzed_lhs = analyze_expression(scope, lhs);
      let analyzed_rhs = analyze_expression(scope, rhs);
      let type_lhs = N.get_type(analyzed_lhs);
      let type_rhs = N.get_type(analyzed_rhs);

      (type_lhs, type_rhs)
      |> Typing.check_binary_operation(op)
      |> Option.iter(S.report_type_err(scope, range));

      (
        (op, analyzed_lhs, analyzed_rhs) |> A.of_binary_op,
        switch (op) {
        | LogicalAnd
        | LogicalOr
        | LessOrEqual
        | LessThan
        | GreaterOrEqual
        | GreaterThan
        | Equal
        | Unequal => Valid(`Boolean)

        | Divide
        | Exponent => Valid(`Float)

        | Add
        | Subtract
        | Multiply =>
          switch (type_lhs, type_rhs) {
          | (Valid(`Integer), Valid(`Integer)) => Valid(`Integer)

          | (_, Valid(`Float))
          | (Valid(`Float), _) => Valid(`Float)

          /* forward invalid types */
          | (Invalid(_), _) => type_lhs
          | (_, Invalid(_)) => type_rhs

          | _ => Invalid(NotInferrable)
          }
        },
      );

    | DotAccess(expr, prop) =>
      let analyzed_expr = analyze_expression(scope, expr);
      let type_expr = N.get_type(analyzed_expr);

      type_expr
      |> Typing.check_dot_access(NR.get_value(prop))
      |> Option.iter(S.report_type_err(scope, range));

      (
        (analyzed_expr, prop) |> A.of_dot_access,
        (
          switch (type_expr) {
          | Valid(`Struct(props)) =>
            props
            |> List.find_opt(fst % (==)(NR.get_value(prop)))
            |> Option.map(snd)
          | _ => None
          }
        )
        |?: Invalid(NotInferrable),
      );

    | FunctionCall(expr, args) =>
      let analyzed_expr = analyze_expression(scope, expr);
      let analyzed_args = args |> List.map(analyze_expression(scope));
      let type_expr = N.get_type(analyzed_expr);
      let type_args = analyzed_args |> List.map(N.get_type);

      (type_expr, type_args)
      |> Typing.check_function_call
      |> Option.iter(S.report_type_err(scope, range));

      (
        (analyzed_expr, analyzed_args) |> A.of_func_call,
        switch (type_expr) {
        | Valid(`Function(args, result)) => result
        | _ => Invalid(NotInferrable)
        },
      );
    };

  N.create(expr, type_, range);
}

and analyze_statement =
    (scope: S.t, (raw_stmt, _, range): AR.statement_t): A.statement_t => {
  let (stmt, type_) =
    switch (raw_stmt) {
    | Variable(id, expr) =>
      let analyzed = analyze_expression(scope, expr);
      let type_ = N.get_type(analyzed);

      scope
      |> S.define(NR.get_value(id), type_)
      |> Option.iter(S.report_type_err(scope, NR.get_range(id)));

      ((id, analyzed) |> A.of_var, T.Valid(`Nil));

    | Expression(expr) =>
      let analyzed = analyze_expression(scope, expr);

      (A.of_expr(analyzed), N.get_type(analyzed));
    };

  N.create(stmt, type_, range);
};

let analyze_argument =
    (scope: S.t, (raw_arg, _, range): AR.argument_t): A.argument_t => {
  let (arg, type_) =
    switch (raw_arg) {
    | {name, default: None, type_: None} =>
      T.UntypedFunctionArgument(NR.get_value(name))
      |> S.report_type_err(scope, range);

      (A.{name, default: None, type_: None}, T.Invalid(NotInferrable));

    | {name, default: Some(raw_expr), type_: None} =>
      let expr = raw_expr |> analyze_expression(scope);

      (A.{name, default: Some(expr), type_: None}, N.get_type(expr));

    | {name, default: None, type_: Some(type_expr)} =>
      let type_ = type_expr |> NR.get_value |> Typing.eval_type_expression;

      (A.{name, default: None, type_: Some(type_expr)}, type_);

    | {name, default: Some(raw_expr), type_: Some(type_expr)} =>
      let expr = raw_expr |> analyze_expression(scope);
      let expr_type = N.get_type(expr);
      let type_ = type_expr |> NR.get_value |> Typing.eval_type_expression;

      switch (expr_type, type_) {
      | (Valid(_), Valid(_)) when expr_type != type_ =>
        T.TypeMismatch(type_, expr_type)
        |> S.report_type_err(scope, N.get_range(expr))

      /* ignore cases where either type is invalid or when types are equal */
      | _ => ()
      };

      (A.{name, default: Some(expr), type_: Some(type_expr)}, type_);
    };

  N.create(arg, type_, range);
};

let rec _check_default_arguments =
        (~require_default=false, scope: S.t, args: list(A.argument_t)) =>
  switch (args, require_default) {
  | ([], _) => ()

  | ([(A.{name, default: None}, _, range), ...xs], true) =>
    Type.DefaultArgumentMissing(NR.get_value(name))
    |> S.report_type_err(scope, range);

    _check_default_arguments(~require_default, scope, xs);

  | ([(A.{default: Some(_)}, _, _), ...xs], _) =>
    _check_default_arguments(~require_default=true, scope, xs)

  | ([x, ...xs], _) => _check_default_arguments(~require_default, scope, xs)
  };

let analyze_argument_list =
    (scope: S.t, raw_args: list(AR.argument_t)): list(A.argument_t) => {
  let args = raw_args |> List.map(analyze_argument(scope));

  _check_default_arguments(scope, args);

  args;
};

let analyze_view_body =
    (scope: S.t, raw_body: AR.expression_t): A.expression_t => {
  let body = raw_body |> analyze_expression(scope);
  let type_ = N.get_type(body);

  type_
  |> Typing.check_jsx_primitive_expression
  |> Option.iter(body |> N.get_range |> S.report_type_err(scope));

  body;
};
