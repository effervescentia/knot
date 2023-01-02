open Knot.Kore;
open Common;

module Variant = {
  type t('typ) = (identifier_t, list(Node.t(TypeExpression.node_t, 'typ)));

  type node_t('typ) = Node.t(t('typ), 'typ);
};

module Argument = {
  type t('typ) = (
    identifier_t,
    option(Node.t(TypeExpression.node_t, 'typ)),
    option(Expression.node_t('typ)),
  );

  type node_t('typ) = Node.t(t('typ), 'typ);
};

type t('typ) =
  | Enumerated(list(Variant.node_t('typ)))
  | Constant(Expression.node_t('typ))
  | Function(list(Argument.node_t('typ)), Expression.node_t('typ))
  | View(
      list(Argument.node_t('typ)),
      list(Expression.node_t('typ)),
      Expression.node_t('typ),
    );

type node_t('typ) = Node.t(t('typ), 'typ);
