open Kore;

module Parameter = {
  type t('expr, 'typ) = (
    AST.Common.identifier_t,
    option(TypeExpression.node_t),
    option(Node.t('expr, 'typ)),
  );

  type node_t('expr, 'typ) = Node.t(t('expr, 'typ), 'typ);
};
