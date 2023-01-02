open Knot.Kore;
open Common;

type t('typ) = list(ModuleStatement.node_t('typ));

type untyped_t = t(unit);
type typed_t = t(Type.t);
