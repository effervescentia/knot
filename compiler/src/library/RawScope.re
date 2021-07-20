open Infix;
open Reference;

exception AnonymousTypeNotFound;

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
 define a new raw type within the scope
 */
let define = (name: Identifier.t, type_: Type.t, scope: t) =>
  Hashtbl.replace(scope.raw, Named(name), type_);

/**
 find the type of an export from a different module
 */
let lookup = (namespace: Namespace.t, id: Export.t, scope: t) => {
  let type_err = Type.ExternalNotFound(namespace, id);

  switch (ModuleTable.find(namespace, scope.modules)) {
  | Some({types}) =>
    switch (Hashtbl.find_opt(types, id)) {
    | Some(t) => Ok(t)
    | None => Error(type_err)
    }
  | None => Error(type_err)
  };
};

/**
 create a new weak anonymous type and add it to the local scope
 */
let weak = (scope: t): Type.t => {
  let seed = scope.seed;
  let type_ = Type.K_Weak(seed);

  Hashtbl.add(scope.anon, seed, Ok(Type.K_Unknown));
  scope.seed = seed + 1;

  type_;
};

/**
 update the constraints on a weak anonymous type
 */
let infer = (id: int, new_trait: Type.trait_t, scope: t) =>
  switch (Hashtbl.find_opt(scope.anon, id)) {
  | Some(Ok(existing_trait)) =>
    switch (Type.restrict(existing_trait, new_trait)) {
    | Some(trait) => Hashtbl.replace(scope.anon, id, Ok(trait))
    | None =>
      Hashtbl.replace(
        scope.anon,
        id,
        Error(TraitConflict(existing_trait, new_trait)),
      )
    }
  | None => Hashtbl.replace(scope.anon, id, Ok(new_trait))
  | _ => ()
  };

/**
 lift a weak anonymous type from a lower scope
 provides a unique id if still an anonymous type
 */
let lift = (id: int, source: t, target: t) =>
  switch (Hashtbl.find_opt(source.anon, id)) {
  | Some(Ok(K_Exactly(t))) =>
    let seed = target.seed;
    target.seed = seed + 1;

    Type.K_Strong(t);
  | Some(Ok(trait)) =>
    let seed = target.seed;
    target.seed = seed + 1;

    Type.K_Strong(K_Anonymous(seed, trait));
  /* TODO: move this function up so invalid type can be reported by context */
  | Some(Error(err)) => Type.K_Invalid(err)
  | None => raise(AnonymousTypeNotFound)
  };
