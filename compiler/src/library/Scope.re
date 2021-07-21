open Infix;
open Reference;

exception AnonymousTypeNotFound;

/**
 container for closure definitions while parsing
 */
type t = {
  /* seed for new anonymous types */
  seed: ref(int),
  modules: ModuleTable.t(Type.t),
  types: Hashtbl.t(Export.t, Type.t),
  anonymous:
    Hashtbl.t(int, result(Type.trait_t, (Type.trait_t, Type.trait_t))),
};

/* static */

let create =
    (
      ~parent=?,
      ~seed=ref(0),
      ~anonymous=Hashtbl.create(0),
      ~modules=ModuleTable.create(0),
      (),
    )
    : t => {
  seed,
  anonymous,
  modules,
  types:
    switch (parent) {
    | Some(parent) => Hashtbl.copy(parent.types)
    | None => Hashtbl.create(1)
    },
};

/* methods */

let clone = (parent: t): t => {
  ...parent,
  anonymous: Hashtbl.copy(parent.anonymous),
  types: Hashtbl.copy(parent.types),
};

/**
 define a new variable within the scope
 */
let define = (name: Identifier.t, type_: Type.t, scope: t) =>
  Hashtbl.replace(scope.types, Named(name), type_);

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
  let seed = scope.seed^;
  let type_ = Type.K_Weak(seed);

  Hashtbl.add(scope.anonymous, seed, Ok(Type.K_Unknown));
  scope.seed := seed + 1;

  type_;
};

/**
 update a weak anonymous type
 */
let infer = (id: int, new_trait: Type.trait_t, scope: t) =>
  switch (Hashtbl.find_opt(scope.anonymous, id)) {
  | Some(Ok(existing_trait)) =>
    switch (Type.restrict(existing_trait, new_trait)) {
    | Some(trait) => Hashtbl.replace(scope.anonymous, id, Ok(trait))
    | None =>
      Hashtbl.replace(
        scope.anonymous,
        id,
        Error((existing_trait, new_trait)),
      )
    }
  | None => Hashtbl.replace(scope.anonymous, id, Ok(new_trait))
  | _ => ()
  };

/**
 lift a weak anonymous type from a lower scope
 provides a unique id if still an anonymous type
 */
let lift = (id: int, source: t, target: t) =>
  switch (Hashtbl.find_opt(source.anonymous, id)) {
  | Some(Ok(K_Exactly(t))) =>
    let seed = target.seed^;
    target.seed := seed + 1;

    Type.K_Strong(t);
  | Some(Ok(trait)) =>
    let seed = target.seed^;
    target.seed := seed + 1;

    Type.K_Strong(K_Anonymous(seed, trait));
  /* TODO: move this function up so invalid type can be reported by context */
  | Some(Error((x, y))) => Type.K_Invalid(TraitConflict(x, y))
  | None => raise(AnonymousTypeNotFound)
  };
