open Kore;
open AST;
open Reference;
open Type.Raw;

let _bind_node = (range: Range.t, x) => (x, range);
let _bind_typed_node = (range: Range.t, (x, y)) => (x, y, range);

let _is_boolean = (==)(`Boolean);
let _is_numeric =
  fun
  | `Integer
  | `Float => true
  | _ => false;

let analyze_primitive =
    ((prim, range): Raw.primitive_t): Analyzed.primitive_t =>
  (
    switch (prim) {
    | Raw.Nil => (Analyzed.Nil, Strong(`Nil))
    | Raw.Boolean(bool) => (Analyzed.Boolean(bool), Strong(`Boolean))
    | Raw.Number(Integer(_) as int) => (
        Analyzed.Number(int),
        Strong(`Integer),
      )
    | Raw.Number(Float(_) as float) => (
        Analyzed.Number(float),
        Strong(`Float),
      )
    | Raw.String(str) => (Analyzed.String(str), Strong(`String))
    }
  )
  |> _bind_typed_node(range);

let rec analyze_statement =
        (scope: Scope.t, (stmt, range): Raw.statement_t)
        : Analyzed.statement_t =>
  (
    switch (stmt) {
    | Raw.Variable(id, expr) => (
        Analyzed.Variable(id, analyze_expression(scope, expr)),
        Strong(`Nil),
      )

    | Raw.Expression(expr) =>
      analyze_expression(scope, expr)
      |> (x => (Analyzed.Expression(x), Node.get_type(x)))
    }
  )
  |> _bind_typed_node(range)

and analyze_expression =
    (scope: Scope.t, (expr, range): Raw.expression_t): Analyzed.expression_t =>
  (
    switch (expr) {
    | Raw.Primitive(prim) =>
      analyze_primitive(prim)
      |> (x => (Analyzed.Primitive(x), Node.get_type(x)))

    | Raw.JSX(jsx) =>
      analyze_jsx(analyze_expression(scope), jsx)
      |> (x => (Analyzed.JSX(x), Node.get_type(x)))

    | Raw.Group(expr) =>
      analyze_expression(scope, expr)
      |> (x => (Analyzed.Group(x), Node.get_type(x)))

    | Raw.Closure(stmts) =>
      stmts
      |> List.map(analyze_statement(Scope.create_child(scope, range)))
      |> (
        xs => (
          Analyzed.Closure(xs),
          xs
          |> List.last
          |> Option.map(
               Node.get_value
               % (
                 fun
                 | Analyzed.Expression(expr) => Node.get_type(expr)
                 /* variable declarations result in a `nil` value */
                 | Analyzed.Variable(_) => Strong(`Nil)
               ),
             )
          /* empty closures result in a `nil` value */
          |?: Strong(`Nil),
        )
      )

    | Raw.Identifier((name, range) as id) =>
      let type_ = scope |> Scope.resolve(id);

      (Analyzed.Identifier((name, type_, range)), type_);

    | Raw.UnaryOp((Negative | Positive) as op, expr) =>
      let x = analyze_expression(scope, expr);
      let (type_, _) =
        scope
        |> Scope.test_and_narrow(
             _is_numeric,
             /* TODO: narrows the type to an integer for now, use numeric trait when introduced */
             `Integer,
             x,
           );

      (Analyzed.UnaryOp(op, x), type_);

    | Raw.UnaryOp(Not as op, expr) =>
      let x = analyze_expression(scope, expr);
      let (type_, _) =
        scope |> Scope.test_and_narrow(_is_boolean, `Boolean, x);

      (Analyzed.UnaryOp(op, x), type_);

    | Raw.BinaryOp((LogicalAnd | LogicalOr) as op, lhs, rhs) =>
      let l = analyze_expression(scope, lhs);
      let r = analyze_expression(scope, rhs);

      scope |> Scope.test_and_narrow(_is_boolean, `Boolean, l) |> ignore;
      scope |> Scope.test_and_narrow(_is_boolean, `Boolean, r) |> ignore;

      /* logic operations will always result in a boolean type */
      (Analyzed.BinaryOp(op, l, r), Strong(`Boolean));

    | Raw.BinaryOp((Add | Subtract | Multiply) as op, lhs, rhs) =>
      let l = analyze_expression(scope, lhs);
      let r = analyze_expression(scope, rhs);

      /* TODO: narrows the type to an integer for now, use numeric trait when introduced */
      let (l_type, l_actual_type) =
        scope |> Scope.test_and_narrow(_is_numeric, `Integer, l);
      let (r_type, r_actual_type) =
        scope |> Scope.test_and_narrow(_is_numeric, `Integer, r);

      let res_type =
        switch (l_actual_type, r_actual_type) {
        /* if both are integers then the resulting type is also an integer */
        | (Some(`Integer), Some(`Integer)) => Strong(`Integer)

        /* if either is a float then the resulting type must be a float */
        | (Some(`Float), _)
        | (_, Some(`Float)) => Strong(`Float)

        | _
            when
              switch (l_type, r_type) {
              /* fall back to integer when either type is invalid */
              | (Invalid(_), _)
              | (_, Invalid(_)) => true

              | _ => false
              } =>
          Strong(`Integer)

        | (Some(lt), Some(rt)) =>
          scope
          |> Scope.report_raw(TypeMismatch(Strong(lt), Strong(rt)), range)

        /* fall back to integer when either type is invalid */
        | _ => Strong(`Integer)
        };

      (Analyzed.BinaryOp(op, l, r), res_type);

    | Raw.BinaryOp((Divide | Exponent) as op, lhs, rhs) =>
      let l = analyze_expression(scope, lhs);
      let r = analyze_expression(scope, rhs);

      /* TODO: narrows the type to an integer for now, use numeric trait when introduced */
      scope |> Scope.test_and_narrow(_is_numeric, `Integer, l) |> ignore;
      scope |> Scope.test_and_narrow(_is_numeric, `Integer, r) |> ignore;

      /* divide and exponent will always result in a float type */
      (Analyzed.BinaryOp(op, l, r), Strong(`Float));

    | Raw.BinaryOp(
        (LessOrEqual | LessThan | GreaterOrEqual | GreaterThan) as op,
        lhs,
        rhs,
      ) =>
      let l = analyze_expression(scope, lhs);
      let r = analyze_expression(scope, rhs);

      /* TODO: narrows the type to an integer for now, use numeric trait when introduced */
      scope |> Scope.test_and_narrow(_is_numeric, `Integer, l) |> ignore;
      scope |> Scope.test_and_narrow(_is_numeric, `Integer, r) |> ignore;

      (Analyzed.BinaryOp(op, l, r), Strong(`Boolean));

    | Raw.BinaryOp((Equal | Unequal) as op, lhs, rhs) =>
      let l = analyze_expression(scope, lhs);
      let r = analyze_expression(scope, rhs);

      /* only analyze types when types do not match */
      let res_type =
        /* guarantee we these aren't the same weak type */
        if (Node.get_type(l) != Node.get_type(r)) {
          /* FIXME: these cases are complex, it would be nice if it could be more clear */
          switch (
            scope |> Scope.resolve_type(l),
            scope |> Scope.resolve_type(r),
          ) {
          /* return the left type when equal */
          | (Strong(lt) | Weak(_, _, lt), Strong(rt) | Weak(_, _, rt))
              when lt == rt =>
            Node.get_type(l)

          /*
            TODO: this can likely be improved as these generic types may be
            narrowed to the same type later in the type analysis
           */
          /* throw error on different generic types */
          | (Weak(_, _, `Generic(_) as lt), Weak(_, _, `Generic(_) as rt)) =>
            scope
            |> Scope.report_raw(
                 Type.Error.TypeMismatch(Strong(lt), Strong(rt)),
                 range,
               )

          /* strong types narrow weak generic types */
          | (Strong(t), Weak(weak_scope, weak_id, `Generic(_)))
          | (Weak(weak_scope, weak_id, `Generic(_)), Strong(t)) =>
            Hashtbl.replace(weak_scope.weak_types, weak_id, Ok(t));

            Type.Raw.Strong(t);

          /* throw error on other different types */
          | (Strong(lt) | Weak(_, _, lt), Strong(rt) | Weak(_, _, rt)) =>
            scope
            |> Scope.report_raw(
                 Type.Error.TypeMismatch(Strong(lt), Strong(rt)),
                 range,
               )

          /* throw error on invalid right type */
          | (Strong(t) | Weak(_, _, t), Invalid(err)) =>
            scope
            |> Scope.report_raw(
                 Type.Error.TypeMismatch(Strong(t), Invalid(err)),
                 range,
               )

          /* fallback to left type error */
          | (Invalid(_), _) => Node.get_type(l)
          };
        } else {
          Node.get_type(l);
        };

      (Analyzed.BinaryOp(op, l, r), res_type);
    }
  )
  |> _bind_typed_node(range)

and analyze_jsx =
    (
      analyze_expression: Raw.expression_t => Analyzed.expression_t,
      (jsx, range): Raw.jsx_t,
    )
    : Analyzed.jsx_t =>
  (
    switch (jsx) {
    | Raw.Tag(id, attrs, children) => (
        Analyzed.Tag(
          id,
          attrs |> List.map(analyze_jsx_attribute(analyze_expression)),
          children |> List.map(analyze_jsx_child(analyze_expression)),
        ),
        Strong(`Element),
      )

    | Raw.Fragment(children) => (
        Analyzed.Fragment(
          children |> List.map(analyze_jsx_child(analyze_expression)),
        ),
        Strong(`Element),
      )
    }
  )
  |> _bind_typed_node(range)

and analyze_jsx_attribute =
    (
      analyze_expression: Raw.expression_t => Analyzed.expression_t,
      (attr, range): Raw.jsx_attribute_t,
    )
    : Analyzed.jsx_attribute_t =>
  (
    switch (attr) {
    | Raw.ID(id) => (Analyzed.ID(id), Strong(`String))

    | Raw.Class(id, expr) => (
        Analyzed.Class(id, expr |> Option.map(analyze_expression)),
        Strong(`String),
      )

    | Raw.Property(id, expr) =>
      expr
      |> Option.map(analyze_expression)
      |> (
        x => (
          Analyzed.Property(id, x),
          x |> Option.map(Node.get_type) |?: Strong(`Generic((0, 0))),
        )
      )
    }
  )
  |> _bind_typed_node(range)

and analyze_jsx_child =
    (
      analyze_expression: Raw.expression_t => Analyzed.expression_t,
      (attr, range): Raw.jsx_child_t,
    )
    : Analyzed.jsx_child_t =>
  (
    switch (attr) {
    | Raw.Text(text) => (
        Analyzed.Text((
          Node.Raw.get_value(text),
          Strong(`String),
          Node.Raw.get_range(text),
        )),
        Strong(`String),
      )

    | Raw.Node(jsx) => (
        Analyzed.Node(analyze_jsx(analyze_expression, jsx)),
        Strong(`Element),
      )

    | Raw.InlineExpression(expr) =>
      analyze_expression(expr)
      |> (x => (Analyzed.InlineExpression(x), Node.get_type(x)))
    }
  )
  |> _bind_typed_node(range);

let analyze_arg =
    (scope: Scope.t, (arg, range): Raw.argument_t): Analyzed.argument_t => {
  let name = arg.name;
  let default = arg.default |> Option.map(analyze_expression(scope));
  let initial_type = default |> Option.map(Node.get_type) |?: Weak(0, 0);

  scope |> Scope.define(name, initial_type);

  ({name, default, type_: None}, initial_type, range);
};
