open Knot.Kore;

module Plugin =
  AST.Framework.Statement.MakeTypes({
    type value_t('expr, 'typ) = Node.t('expr, 'typ);
  });
