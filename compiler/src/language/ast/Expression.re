open Knot.Kore;
open Common;

module StyleRule = {
  type t('expr, 'typ) = (Node.t(string, 'typ), Node.t('expr, 'typ));

  type node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};

/**
   an expression AST node
   */
type expression_t('a) = Node.t(raw_expression_t('a), 'a)
/**
   supported expressions and type containers
   */
and raw_expression_t('a) =
  | Primitive(Primitive.t)
  | Identifier(string)
  | KSX(KSX.t(raw_expression_t('a), 'a))
  | Group(expression_t('a))
  | BinaryOp(Operator.Binary.t, expression_t('a), expression_t('a))
  | UnaryOp(Operator.Unary.t, expression_t('a))
  | Closure(list(Statement.node_t(raw_expression_t('a), 'a)))
  | DotAccess(expression_t('a), identifier_t)
  | BindStyle(KSX.ViewKind.t, expression_t('a), expression_t('a))
  | FunctionCall(expression_t('a), list(expression_t('a)))
  | Style(list(StyleRule.node_t(raw_expression_t('a), 'a)));

/**
   an AST parameter node of a functional closure
   */
type parameter_t('a) = Node.t(raw_parameter_t('a), 'a)
/**
   a parameter node of a functional closure
   */
and raw_parameter_t('a) = (
  identifier_t,
  option(TypeExpression.t),
  option(expression_t('a)),
);
