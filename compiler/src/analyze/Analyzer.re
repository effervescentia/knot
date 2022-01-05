open Kore;
open AST;
open Reference;
open Type.Raw;

let _bind_node = (range: Range.t, x) => (x, range);
let _bind_typed_node = (range: Range.t, (x, y)) => (x, y, range);

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

      (
        Analyzed.UnaryOp(op, x),
        scope
        |> Scope.infer(
             fun
             | `Integer
             | `Float => true
             | _ => false,
             /* TODO: narrows the type to an integer for now, use numeric trait when introduced */
             _ => `Integer,
             x,
           ),
      );

    | Raw.UnaryOp(Not as op, expr) =>
      let x = analyze_expression(scope, expr);

      (
        Analyzed.UnaryOp(op, x),
        scope |> Scope.infer((==)(`Boolean), _ => `Boolean, x),
      );

    | Raw.BinaryOp(op, lhs, rhs) => (
        Analyzed.BinaryOp(
          op,
          analyze_expression(scope, lhs),
          analyze_expression(scope, rhs),
        ),
        /* TODO: implement */
        Strong(`Generic((0, 0))),
      )
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
