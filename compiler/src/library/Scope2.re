open Infix;
open Reference;

type t = {
  mutable seed: int,
  id: int,
  raw: Hashtbl.t(Export.t, Type.t),
  anon: Hashtbl.t(int, result(Type.trait_t, Type.type_err)),
  final: Hashtbl.t(Export.t, Type2.t),
  modules: ModuleTable.t,
  parent: option(t),
};

/* static */

let create = (~parent=?, ~modules=ModuleTable.create(0), ()): t => {
  id: parent |?> (x => x.id) |?: 0,
  seed: 0,
  raw: Hashtbl.create(0),
  anon: Hashtbl.create(0),
  final: Hashtbl.create(0),
  parent,
  modules,
};

/* methods */

let clone = (parent: t): t => create(~parent, ~modules=parent.modules, ());

/**
 define a new variable within the scope
 */
let define_raw = (name: Identifier.t, type_: Type.t, scope: t) =>
  Hashtbl.replace(scope.raw, Named(name), type_);
