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

  type parameter_t = Expression.parameter_t(type_t);
  type raw_parameter_t = Expression.raw_parameter_t(type_t);

  /* tag helpers */

  let of_var = ((name, expression)) =>
    Expression.Variable(name, expression);
  let of_effect = expression => Expression.Effect(expression);
  let of_id = name => Expression.Identifier(name);
  let of_group = expression => Expression.Group(expression);
  let of_closure = statements => Expression.Closure(statements);
  let of_dot_access = ((expr, prop)) => Expression.DotAccess(expr, prop);
  let of_element_bind_style = ((view, style)) =>
    Expression.BindStyle(Element, view, style);
  let of_component_bind_style = ((view, style)) =>
    Expression.BindStyle(Component, view, style);
  let of_func_call = ((function_, arguments)) =>
    Expression.FunctionCall(function_, arguments);
  let of_style = rules => Expression.Style(rules);

  let of_unary_op = (operator, expression) =>
    Expression.UnaryOp(operator, expression);
  let of_not_op = x => of_unary_op(Not, x);
  let of_neg_op = x => of_unary_op(Negative, x);
  let of_pos_op = x => of_unary_op(Positive, x);

  let of_binary_op = (op, (lhs, rhs)) => Expression.BinaryOp(op, lhs, rhs);
  let of_and_op = x => of_binary_op(LogicalAnd, x);
  let of_or_op = x => of_binary_op(LogicalOr, x);

  let of_mult_op = x => of_binary_op(Multiply, x);
  let of_div_op = x => of_binary_op(Divide, x);
  let of_add_op = x => of_binary_op(Add, x);
  let of_sub_op = x => of_binary_op(Subtract, x);

  let of_lt_op = x => of_binary_op(LessThan, x);
  let of_lte_op = x => of_binary_op(LessOrEqual, x);
  let of_gt_op = x => of_binary_op(GreaterThan, x);
  let of_gte_op = x => of_binary_op(GreaterOrEqual, x);

  let of_eq_op = x => of_binary_op(Equal, x);
  let of_ineq_op = x => of_binary_op(Unequal, x);

  let of_expo_op = x => of_binary_op(Exponent, x);

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
