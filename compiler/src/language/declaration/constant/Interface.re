open Knot.Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) = KExpression.Interface.node_t('typ);
  });
