open Infix;
open Reference;

type t = {
  types: Hashtbl.t(Export.t, Type.t),
  mutable children: list((t, Cursor.t)),
};

/* static */

let create = (~types=Hashtbl.create(0), ()): t => {types, children: []};

/* methods */

let child = (cursor: Cursor.t, scope: t): t => {
  let child_scope = create();
  scope.children = scope.children @ [(child_scope, cursor)];

  child_scope;
};
