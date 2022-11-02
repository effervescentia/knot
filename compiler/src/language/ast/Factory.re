/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Reference;
open Common;

module TypeExpression = TypeExpression;
module TypeDefinition = TypeDefinition;

/**
 abstraction on the type of the nodes that makeup an AST
 */
module type ASTParams = {
  type type_t;

  let node_to_entity:
    (
      ~attributes: list(Pretty.XML.xml_attr_t(string))=?,
      ~children: list(Dump.Entity.t)=?,
      string,
      N.t('a, type_t)
    ) =>
    Dump.Entity.t;
};

/**
 constructor for AST modules
 */
module Make = (Params: ASTParams) => {
  include Common;

  let typed_node_to_entity = Params.node_to_entity;

  /**
   type for AST expression nodes
   */
  type type_t = Params.type_t;

  /**
   container for AST nodes
   */
  type node_t('a) = N.t('a, type_t);

  /**
   a JSX AST node
   */
  type jsx_t =
    | Tag(identifier_t, list(jsx_attribute_t), list(jsx_child_t))
    | Component(node_t(string), list(jsx_attribute_t), list(jsx_child_t))
    | Fragment(list(jsx_child_t))

  /**
   a JSX child AST node
   */
  and jsx_child_t = untyped_t(raw_jsx_child_t)
  /**
   supported JSX children
   */
  and raw_jsx_child_t =
    | Text(string)
    | Node(jsx_t)
    | InlineExpression(expression_t)

  /**
   a JSX attribute AST node
   */
  and jsx_attribute_t = untyped_t(raw_jsx_attribute_t)
  /**
   supported JSX attributes
   */
  and raw_jsx_attribute_t =
    | ID(identifier_t)
    | Class(identifier_t, option(expression_t))
    | Property(identifier_t, option(expression_t))

  /**
   a style AST node
   */
  and style_rule_t = untyped_t(raw_style_rule_t)
  /**
   string key and style expression pair
   */
  and raw_style_rule_t = (Node.t(string, type_t), expression_t)

  /**
   an expression AST node
   */
  and expression_t = node_t(raw_expression_t)
  /**
   supported expressions and type containers
   */
  and raw_expression_t =
    | Primitive(primitive_t)
    | Identifier(string)
    | JSX(jsx_t)
    | Group(expression_t)
    | BinaryOp(binary_t, expression_t, expression_t)
    | UnaryOp(unary_t, expression_t)
    | Closure(list(statement_t))
    | DotAccess(expression_t, untyped_t(string))
    | FunctionCall(expression_t, list(expression_t))
    | Style(list(style_rule_t))

  /**
   a statement AST node
   */
  and statement_t = node_t(raw_statement_t)
  /**
   supported statement types
   */
  and raw_statement_t =
    | Variable(identifier_t, expression_t)
    | Expression(expression_t);

  /**
   an AST node of an argument for a functional closure
   */
  type argument_t = node_t(raw_argument_t)
  /**
   a node of an argument for a functional closure
   */
  and raw_argument_t = {
    name: identifier_t,
    default: option(expression_t),
    type_: option(TypeExpression.t),
  };

  /* tag helpers */

  let of_internal = namespace => Namespace.Internal(namespace);
  let of_external = namespace => Namespace.External(namespace);

  let of_var = ((name, x)) => Variable(name, x);
  let of_expr = x => Expression(x);
  let of_id = x => Identifier(x);
  let of_group = x => Group(x);
  let of_closure = xs => Closure(xs);
  let of_dot_access = ((expr, prop)) => DotAccess(expr, prop);
  let of_func_call = ((expr, args)) => FunctionCall(expr, args);
  let of_style = rules => Style(rules);

  let of_unary_op = ((op, x)) => UnaryOp(op, x);
  let of_not_op = x => (Not, x) |> of_unary_op;
  let of_neg_op = x => (Negative, x) |> of_unary_op;
  let of_pos_op = x => (Positive, x) |> of_unary_op;

  let of_binary_op = ((op, l, r)) => BinaryOp(op, l, r);
  let of_and_op = ((l, r)) => (LogicalAnd, l, r) |> of_binary_op;
  let of_or_op = ((l, r)) => (LogicalOr, l, r) |> of_binary_op;

  let of_mult_op = ((l, r)) => (Multiply, l, r) |> of_binary_op;
  let of_div_op = ((l, r)) => (Divide, l, r) |> of_binary_op;
  let of_add_op = ((l, r)) => (Add, l, r) |> of_binary_op;
  let of_sub_op = ((l, r)) => (Subtract, l, r) |> of_binary_op;

  let of_lt_op = ((l, r)) => (LessThan, l, r) |> of_binary_op;
  let of_lte_op = ((l, r)) => (LessOrEqual, l, r) |> of_binary_op;
  let of_gt_op = ((l, r)) => (GreaterThan, l, r) |> of_binary_op;
  let of_gte_op = ((l, r)) => (GreaterOrEqual, l, r) |> of_binary_op;

  let of_eq_op = ((l, r)) => (Equal, l, r) |> of_binary_op;
  let of_ineq_op = ((l, r)) => (Unequal, l, r) |> of_binary_op;

  let of_expo_op = ((l, r)) => (Exponent, l, r) |> of_binary_op;

  let of_jsx = x => JSX(x);
  let of_frag = xs => Fragment(xs);
  let of_tag = ((name, attrs, children)) => Tag(name, attrs, children);
  let of_component = ((id, attrs, children)) =>
    Component(id, attrs, children);
  let of_prop = ((name, value)) => Property(name, value);
  let of_jsx_class = ((name, value)) => Class(name, value);
  let of_jsx_id = name => ID(name);
  let of_text = x => Text(x);
  let of_node = x => Node(x);
  let of_inline_expr = x => InlineExpression(x);

  let of_prim = x => Primitive(x);
  let of_bool = x => Boolean(x);
  let of_int = x => Integer(x);
  let of_float = ((x, precision)) => Float(x, precision);
  let of_string = x => String(x);
  let of_num = x => Number(x);
  let nil = Nil;

  module Dump = {
    include Common.Dump;

    let rec expr_to_entity = expr =>
      (
        switch (fst(expr)) {
        | Primitive(prim) =>
          typed_node_to_entity(
            ~attributes=[("value", prim_to_string(prim))],
            "Primitive",
          )

        | Identifier(id) =>
          typed_node_to_entity(~attributes=[("value", id)], "Identifier")

        | JSX(jsx) =>
          typed_node_to_entity(~children=[jsx_to_entity(jsx)], "JSX")

        | Group(group) =>
          typed_node_to_entity(~children=[expr_to_entity(group)], "Group")

        | Closure(stmts) =>
          typed_node_to_entity(
            ~children=
              stmts
              |> List.map(stmt =>
                   typed_node_to_entity(
                     ~children=[stmt_to_entity(stmt)],
                     "Statement",
                     stmt,
                   )
                 ),
            "Closure",
          )

        | BinaryOp(op, lhs, rhs) =>
          typed_node_to_entity(
            ~children=[
              typed_node_to_entity(
                ~children=[expr_to_entity(lhs)],
                "LHS",
                lhs,
              ),
              typed_node_to_entity(
                ~children=[expr_to_entity(rhs)],
                "RHS",
                rhs,
              ),
            ],
            binary_to_string(op),
          )

        | UnaryOp(op, expr) =>
          typed_node_to_entity(
            ~children=[expr_to_entity(expr)],
            unary_to_string(op),
          )

        | DotAccess(expr, (prop, _)) =>
          typed_node_to_entity(
            ~attributes=[("name", prop)],
            ~children=[expr_to_entity(expr)],
            "DotAccess",
          )

        | FunctionCall(expr, args) =>
          typed_node_to_entity(
            ~children=[
              Entity.create(~children=[expr_to_entity(expr)], "Function"),
              Entity.create(
                ~children=args |> List.map(expr_to_entity),
                "Arguments",
              ),
            ],
            "FunctionCall",
          )

        | Style(rules) =>
          typed_node_to_entity(
            ~children=
              rules
              |> List.map((((key, value), _) as rule) =>
                   untyped_node_to_entity(
                     ~children=[
                       typed_node_to_entity(
                         ~attributes=[("value", fst(key))],
                         "Key",
                         key,
                       ),
                       Entity.create(
                         ~children=[expr_to_entity(value)],
                         "Value",
                       ),
                     ],
                     "Rule",
                     rule,
                   )
                 ),
            "Style",
          )
        }
      )(
        expr,
      )

    and jsx_to_entity =
      fun
      | Tag(name, attrs, children) =>
        Entity.create(
          ~children=[
            id_to_entity("Name", name),
            Entity.create(
              ~children=attrs |> List.map(jsx_attr_to_entity),
              "Attributes",
            ),
            Entity.create(
              ~children=children |> List.map(jsx_child_to_entity),
              "Children",
            ),
          ],
          "Tag",
        )

      | Component(view, attrs, children) =>
        Entity.create(
          ~children=[
            view
            |> typed_node_to_entity(
                 ~attributes=[("name", fst(view))],
                 "Identifier",
               ),
            Entity.create(
              ~children=attrs |> List.map(jsx_attr_to_entity),
              "Attributes",
            ),
            Entity.create(
              ~children=children |> List.map(jsx_child_to_entity),
              "Children",
            ),
          ],
          "Component",
        )

      | Fragment(children) =>
        Entity.create(
          ~children=children |> List.map(jsx_child_to_entity),
          "Fragment",
        )

    and jsx_child_to_entity = jsx_child =>
      (
        switch (fst(jsx_child)) {
        | Text(text) =>
          untyped_node_to_entity(~attributes=[("value", text)], "Text")

        | Node(jsx) =>
          untyped_node_to_entity(~children=[jsx_to_entity(jsx)], "Node")

        | InlineExpression(expr) =>
          untyped_node_to_entity(
            ~children=[expr_to_entity(expr)],
            "InlineExpr",
          )
        }
      )(
        jsx_child,
      )

    and jsx_attr_to_entity = attr =>
      (
        (
          switch (fst(attr)) {
          | Class(name, value) => ("Class", name, value)

          | ID(name) => ("ID", name, None)

          | Property(name, value) => ("Property", name, value)
          }
        )
        |> Tuple.map_snd3(id_to_entity("Name"))
        |> (
          fun
          | (entity, name_child, Some(expr)) =>
            untyped_node_to_entity(
              ~children=[name_child, expr_to_entity(expr)],
              entity,
            )
          | (entity, name_child, None) =>
            untyped_node_to_entity(~children=[name_child], entity)
        )
      )(
        attr,
      )

    and stmt_to_entity = stmt =>
      (
        switch (fst(stmt)) {
        | Variable(name, expr) =>
          typed_node_to_entity(
            "Variable",
            ~children=[id_to_entity("Name", name), expr_to_entity(expr)],
          )

        | Expression(expr) =>
          typed_node_to_entity(
            "Expression",
            ~children=[expr_to_entity(expr)],
          )
        }
      )(
        stmt,
      );
  };
};
