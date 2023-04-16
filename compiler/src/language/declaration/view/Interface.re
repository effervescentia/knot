open Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) = (
      list(Lambda.Parameter.node_t(Expression.t('typ), 'typ)),
      list(Node.t(string, 'typ)),
      Node.t(Expression.t('typ), 'typ),
    );
  });
