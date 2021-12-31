open Kore;
open AST;
open Reference;

type mapping_t = Hashtbl.t((int, int), Type.t);

let _resolve_node =
    (
      mapping: mapping_t,
      resolve_value: 'a => 'b,
      node: Node.t('a, Type.Raw.t),
    )
    : Node.t('b, Type.t) => {
  let raw_type = Node.get_type(node);

  let type_ =
    raw_type
    |> Type.of_raw((scope_id, weak_id) =>
         Hashtbl.find(mapping, (scope_id, weak_id))
       );

  Node.create(
    resolve_value(Node.get_value(node)),
    type_,
    Node.get_range(node),
  );
};

let resolve_primitive =
    (mapping: mapping_t, expr: Analyzed.primitive_t): primitive_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.Nil => Nil
    | Analyzed.Boolean(bool) => Boolean(bool)
    | Analyzed.Number(num) => Number(num)
    | Analyzed.String(str) => String(str),
    expr,
  );

let rec resolve_statement =
        (mapping: mapping_t, stmt: Analyzed.statement_t): statement_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.Expression(expr) =>
      Expression(resolve_expression(mapping, expr))
    | Analyzed.Variable(id, expr) =>
      Variable(id, resolve_expression(mapping, expr)),
    stmt,
  )

and resolve_expression =
    (mapping: mapping_t, expr: Analyzed.expression_t): expression_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.Primitive(prim) =>
      Primitive(resolve_primitive(mapping, prim))
    | Analyzed.JSX(jsx) => JSX(resolve_jsx(mapping, jsx))
    | Analyzed.Group(expr) => Group(resolve_expression(mapping, expr))
    | Analyzed.Closure(stmts) =>
      Closure(stmts |> List.map(resolve_statement(mapping)))
    | Analyzed.Identifier(id) =>
      Identifier(_resolve_node(mapping, Fun.id, id))
    | Analyzed.UnaryOp(op, expr) =>
      UnaryOp(op, resolve_expression(mapping, expr))
    | Analyzed.BinaryOp(op, lhs, rhs) =>
      BinaryOp(
        op,
        resolve_expression(mapping, lhs),
        resolve_expression(mapping, rhs),
      ),
    expr,
  )

and resolve_jsx = (mapping: mapping_t, jsx: Analyzed.jsx_t): jsx_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.Tag(id, attrs, children) =>
      Tag(
        id,
        attrs |> List.map(resolve_jsx_attribute(mapping)),
        children |> List.map(resolve_jsx_child(mapping)),
      )
    | Analyzed.Fragment(children) =>
      Fragment(children |> List.map(resolve_jsx_child(mapping))),
    jsx,
  )

and resolve_jsx_attribute =
    (mapping: mapping_t, child: Analyzed.jsx_attribute_t): jsx_attribute_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.ID(id) => ID(id)
    | Analyzed.Class(id, expr) =>
      Class(id, expr |> Option.map(resolve_expression(mapping)))
    | Analyzed.Property(id, expr) =>
      Property(id, expr |> Option.map(resolve_expression(mapping))),
    child,
  )

and resolve_jsx_child =
    (mapping: mapping_t, child: Analyzed.jsx_child_t): jsx_child_t =>
  _resolve_node(
    mapping,
    fun
    | Analyzed.Text(text) => Text(_resolve_node(mapping, Fun.id, text))
    | Analyzed.Node(jsx) => Node(resolve_jsx(mapping, jsx))
    | Analyzed.InlineExpression(expr) =>
      InlineExpression(resolve_expression(mapping, expr)),
    child,
  );

let resolve_constant =
    (scope: Scope.t, raw_expr: Raw.expression_t): declaration_t => {
  let analyzed_expr = Analyzer.analyze_expression(scope, raw_expr);
  let mapping = Scope.finalize(scope);

  let expr = resolve_expression(mapping, analyzed_expr);

  Node.(create(of_const(expr), get_type(expr), get_range(expr)));
};

let resolve_arg = (mapping: mapping_t, arg: Analyzed.argument_t): argument_t =>
  _resolve_node(
    mapping,
    ({name, default, type_}: Analyzed.raw_argument_t) =>
      {
        name,
        default: default |> Option.map(resolve_expression(mapping)),
        type_: type_ |> Option.map(_resolve_node(mapping, Fun.id)),
      },
    arg,
  );

let resolve_function =
    (
      scope: Scope.t,
      raw_args: list(Raw.argument_t),
      raw_result: Raw.expression_t,
      range: Range.t,
    ) => {
  let analyzed_args = raw_args |> List.map(Analyzer.analyze_arg(scope));
  let analyzed_result = Analyzer.analyze_expression(scope, raw_result);
  let mapping = Scope.finalize(scope);

  let args = analyzed_args |> List.map(resolve_arg(mapping));
  let result = resolve_expression(mapping, analyzed_result);

  Node.create(
    ([], result) |> of_func,
    Type.Valid(
      `Function((
        args
        |> List.map((({name}, type_, _)) =>
             (name |> Node.Raw.get_value |> Identifier.to_string, type_)
           ),
        Node.get_type(result),
      )),
    ),
    range,
  );
};
