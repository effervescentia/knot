open Knot.Kore;
open AST.Common;

type t('typ) =
  | Constant(KConstant.Interface.Plugin.value_t('typ))
  | Enumerated(KEnumerated.Interface.Plugin.value_t('typ))
  | Function(KFunction.Interface.Plugin.value_t('typ))
  | View(KView.Interface.Plugin.value_t('typ));

type node_t('typ) = Node.t(t('typ), 'typ);

/* static */

let of_constant = x => Constant(x);
let of_enumerated = x => Enumerated(x);
let of_function = x => Function(x);
let of_view = x => View(x);

/* methods */

let fold = (~constant, ~enumerated, ~function_, ~view) =>
  fun
  | Constant(x) => constant(x)
  | Enumerated(x) => enumerated(x)
  | Function(x) => function_(x)
  | View(x) => view(x);
