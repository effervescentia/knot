open Knot.Kore;

module Plugin =
  AST.Framework.Statement.MakeTypes({
    type value_t('expr, 'typ) = (
      AST.Common.identifier_t,
      Node.t('expr, 'typ),
    );
  });
