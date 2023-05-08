open Kore;

module Plugin =
  AST.Framework.Declaration.MakeTypes({
    type value_t('typ) =
      list(
        (AST.Common.identifier_t, list(Node.t(TypeExpression.t, 'typ))),
      );
  });
