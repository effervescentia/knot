open Knot.Kore;

type t('typ) =
  list(
    ModuleStatement.node_t(Declaration.node_t(Expression.t('typ), 'typ)),
  );

type program_t = t(Type.t);
