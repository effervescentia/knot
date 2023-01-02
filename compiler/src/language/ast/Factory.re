/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;

/**
 abstraction on the type of the nodes that makeup an AST
 */
module type ASTParams = {type type_t;};

/**
 constructor for AST modules
 */
module Make = (Params: ASTParams) => {
  include Common;

  /**
   type for AST expression nodes
   */
  type type_t = Params.type_t;

  /**
   container for AST nodes
   */
  type node_t('a) = Node.t('a, type_t);

  type ksx_t = Expression.ksx_t(type_t);

  type ksx_child_t = Expression.ksx_child_t(type_t);
  type raw_ksx_child_t = Expression.raw_ksx_child_t(type_t);

  type ksx_attribute_t = Expression.ksx_attribute_t(type_t);
  type raw_ksx_attribute_t = Expression.raw_ksx_attribute_t(type_t);

  type style_rule_t = Expression.style_rule_t(type_t);
  type raw_style_rule_t = Expression.raw_style_rule_t(type_t);

  type expression_t = Expression.expression_t(type_t);
  type raw_expression_t = Expression.raw_expression_t(type_t);

  type statement_t = Expression.statement_t(type_t);
  type raw_statement_t = Expression.raw_statement_t(type_t);

  type argument_t = Expression.argument_t(type_t);
  type raw_argument_t = Expression.raw_argument_t(type_t);

  /* tag helpers */

  let of_internal = namespace => Reference.Namespace.Internal(namespace);
  let of_external = namespace => Reference.Namespace.External(namespace);

  let of_var = ((name, x)) => Expression.Variable(name, x);
  let of_effect = x => Expression.Effect(x);
  let of_id = x => Expression.Identifier(x);
  let of_group = x => Expression.Group(x);
  let of_closure = xs => Expression.Closure(xs);
  let of_dot_access = ((expr, prop)) => Expression.DotAccess(expr, prop);
  let of_element_bind_style = ((view, style)) =>
    Expression.BindStyle(Element, view, style);
  let of_component_bind_style = ((view, style)) =>
    Expression.BindStyle(Component, view, style);
  let of_func_call = ((expr, args)) => Expression.FunctionCall(expr, args);
  let of_style = rules => Expression.Style(rules);

  let of_unary_op = ((op, x)) => Expression.UnaryOp(op, x);
  let of_not_op = x => (Not, x) |> of_unary_op;
  let of_neg_op = x => (Negative, x) |> of_unary_op;
  let of_pos_op = x => (Positive, x) |> of_unary_op;

  let of_binary_op = ((op, l, r)) => Expression.BinaryOp(op, l, r);
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

  let of_ksx = x => Expression.KSX(x);
  let of_frag = xs => Expression.Fragment(xs);
  let of_element_tag = ((name, styles, attrs, children)) =>
    Expression.Tag(Element, name, styles, attrs, children);
  let of_component_tag = ((name, styles, attrs, children)) =>
    Expression.Tag(Component, name, styles, attrs, children);
  let of_text = x => Expression.Text(x);
  let of_node = x => Expression.Node(x);
  let of_inline_expr = x => Expression.InlineExpression(x);

  let of_prim = x => Expression.Primitive(x);
  let of_bool = x => Boolean(x);
  let of_int = x => Integer(x);
  let of_float = ((x, precision)) => Float(x, precision);
  let of_string = x => String(x);
  let nil = Nil;
};
