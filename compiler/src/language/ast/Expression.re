open Knot.Kore;
open Common;

module ViewKind = {
  type t =
    | Component
    | Element;

  let to_string =
    fun
    | Component => "Component"
    | Element => "Element";
};

/**
   a JSX AST node
   */
type ksx_t('a) =
  | Tag(
      ViewKind.t,
      Node.t(string, 'a),
      list(expression_t('a)),
      list(ksx_attribute_t('a)),
      list(ksx_child_t('a)),
    )
  | Fragment(list(ksx_child_t('a)))

/**
   a JSX child AST node
   */
and ksx_child_t('a) = raw_t(raw_ksx_child_t('a))
/**
   supported JSX children
   */
and raw_ksx_child_t('a) =
  | Text(string)
  | Node(ksx_t('a))
  | InlineExpression(expression_t('a))

/**
   a JSX attribute AST node
   */
and ksx_attribute_t('a) = raw_t((identifier_t, option(expression_t('a))))

and raw_ksx_attribute_t('a) = (identifier_t, option(expression_t('a)))

/**
   a style AST node
   */
and style_rule_t('a) = raw_t(raw_style_rule_t('a))
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
  | KSX(ksx_t('a))
  | Group(expression_t('a))
  | BinaryOp(Binary.t, expression_t('a), expression_t('a))
  | UnaryOp(Unary.t, expression_t('a))
  | Closure(list(statement_t('a)))
  | DotAccess(expression_t('a), identifier_t)
  | BindStyle(ViewKind.t, expression_t('a), expression_t('a))
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
  | Effect(expression_t('a));

/**
   an AST node of an argument for a functional closure
   */
type parameter_t('a) = Node.t(raw_parameter_t('a), 'a)
/**
   a node of an argument for a functional closure
   */
and raw_parameter_t('a) = (
  identifier_t,
  option(TypeExpression.t),
  option(expression_t('a)),
);
