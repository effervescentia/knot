open Reference;

type t = {
  scope: Hashtbl.t(Export.t, Type2.t),
  mutable children: list(t),
};

/* static */

let create = (~scope=Hashtbl.create(0), ()): t => {
  scope,
  children: [],
};

/* methods */
