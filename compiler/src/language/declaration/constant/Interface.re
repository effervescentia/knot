open Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) = Expression.node_t('typ);
  });
