open Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) = (
      list(Lambda.Parameter.node_t(Expression.t('typ), 'typ)),
      Expression.node_t('typ),
    );
  });
