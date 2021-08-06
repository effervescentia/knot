open Reference;

type t = {
  scope: Hashtbl.t(Export.t, Type.t),
  mutable children: list((t, Range.t)),
};

/* static */

let create = (~scope=Hashtbl.create(0), ()): t => {scope, children: []};

let of_seq = (seq: Seq.t((Export.t, Type.t))): t =>
  create(~scope=Hashtbl.of_seq(seq), ());

/* methods */

let add = (key: Export.t, type_: Type.t, tbl: t): unit =>
  Hashtbl.add(tbl.scope, key, type_);
