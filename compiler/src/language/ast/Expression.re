open Knot.Kore;
open Common;

/**
   a JSX AST node
   */
type jsx_t('a) =
  | Tag(identifier_t, list(jsx_attribute_t('a)), list(jsx_child_t('a)))
  | Component(
      Node.t(string, 'a),
      list(jsx_attribute_t('a)),
      list(jsx_child_t('a)),
    )
  | Fragment(list(jsx_child_t('a)))

/**
   a JSX child AST node
   */
and jsx_child_t('a) = untyped_t(raw_jsx_child_t('a))
/**
   supported JSX children
   */
and raw_jsx_child_t('a) =
  | Text(string)
  | Node(jsx_t('a))
  | InlineExpression(expression_t('a))

/**
   a JSX attribute AST node
   */
and jsx_attribute_t('a) = untyped_t(raw_jsx_attribute_t('a))
/**
   supported JSX attributes
   */
and raw_jsx_attribute_t('a) =
  | ID(identifier_t)
  | Class(identifier_t, option(expression_t('a)))
  | Property(identifier_t, option(expression_t('a)))

/**
   a style AST node
   */
and style_rule_t('a) = untyped_t(raw_style_rule_t('a))
/**
   string key and style expression pair
   */
and raw_style_rule_t('a) = (Node.t(string, 'a), expression_t('a))

/**
   an expression AST node
   */
and expression_t('a) = Node.t(raw_expression_t('a), 'a)
/**
   supported expressions and type containers
   */
and raw_expression_t('a) =
  | Primitive(primitive_t)
  | Identifier(string)
  | JSX(jsx_t('a))
  | Group(expression_t('a))
  | BinaryOp(Binary.t, expression_t('a), expression_t('a))
  | UnaryOp(Unary.t, expression_t('a))
  | Closure(list(statement_t('a)))
  | DotAccess(expression_t('a), untyped_t(string))
  | FunctionCall(expression_t('a), list(expression_t('a)))
  | Style(list(style_rule_t('a)))

/**
   a statement AST node
   */
and statement_t('a) = Node.t(raw_statement_t('a), 'a)
/**
   supported statement types
   */
and raw_statement_t('a) =
  | Variable(identifier_t, expression_t('a))
  | Expression(expression_t('a));

/**
   an AST node of an argument for a functional closure
   */
type argument_t('a) = Node.t(raw_argument_t('a), 'a)
/**
   a node of an argument for a functional closure
   */
and raw_argument_t('a) = {
  name: identifier_t,
  default: option(expression_t('a)),
  type_: option(TypeExpression.t),
};
