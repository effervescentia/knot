open Knot.Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) = (
      list(
        KLambda.Interface.Parameter.node_t(
          KExpression.Interface.t('typ),
          'typ,
        ),
      ),
      KExpression.Interface.node_t('typ),
    );
  });
