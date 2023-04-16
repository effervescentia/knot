open Kore;
open AST.Common;

type t('typ) =
  | Constant(Constant.value_t('typ))
  | Enumerated(Enumerated.value_t('typ))
  | Function(Function.value_t('typ))
  | View(View.value_t('typ));

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
