open Knot.Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) =
      list(
        (
          AST.Common.identifier_t,
          list(Node.t(KTypeExpression.Interface.t, 'typ)),
        ),
      );
  });
