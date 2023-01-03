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

module Attribute = {
  type t('expr, 'typ) = (identifier_t, option(Node.t('expr, 'typ)));

  type node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};

module Child = {
  type t('expr, 'ksx, 'typ) =
    | Text(string)
    | Node('ksx)
    | InlineExpression(Node.t('expr, 'typ));

  type node_t('expr, 'ksx, 'typ) = raw_t(t('expr, 'ksx, 'typ));
};

/**
   a JSX AST node
   */
type t('expr, 'typ) =
  | Tag(
      ViewKind.t,
      Node.t(string, 'typ),
      list(Node.t('expr, 'typ)),
      list(Attribute.node_t('expr, 'typ)),
      list(Child.node_t('expr, t('expr, 'typ), 'typ)),
    )
  | Fragment(list(Child.node_t('expr, t('expr, 'typ), 'typ)));
