open Kore;
open Reference;

type t = {
  namespace: Namespace.t,
  parent: option(t),
  types: Hashtbl.t(string, Type.Raw.t),
  /* error reporting callback */
  report: compile_err => unit,
  mutable children: list((t, Range.t)),
};

/* static */

let create =
    (
      ~parent: option(t)=?,
      namespace: Namespace.t,
      report: compile_err => unit,
    )
    : t => {
  namespace,
  parent,
  types: Hashtbl.create(0),
  report,
  children: [],
};

/* methods */

let child = (parent: t, range: Range.t): t => {
  let child = create(~parent, parent.namespace, parent.report);

  parent.children = parent.children @ [(child, range)];

  child;
};

let rec exists = (name: string, scope: t): bool =>
  Hashtbl.mem(scope.types, name)
  || (
    switch (scope.parent) {
    | Some(parent) => parent |> exists(name)
    | _ => false
    }
  );

let rec resolve =
        ((name, range) as id: Node.Raw.t(string), scope: t)
        : option(Type.Raw.t) =>
  switch (Hashtbl.find_opt(scope.types, Node.Raw.value(id)), scope.parent) {
  | (Some(_) as result, _) => result
  | (_, Some(parent)) => parent |> resolve(id)
  | _ =>
    let type_err =
      Type.Error.DuplicateIdentifier(Identifier.of_string(name));

    scope.report(ParseError(TypeError(type_err), scope.namespace, range));

    Some(Invalid(type_err));
  };

let _safe_define =
    ((name, range): Node.Raw.t(string), type_: Type.Raw.t, scope: t) =>
  (
    if (scope |> exists(name)) {
      let type_err =
        Type.Error.DuplicateIdentifier(Identifier.of_string(name));

      scope.report(ParseError(TypeError(type_err), scope.namespace, range));

      Type.Raw.Invalid(type_err);
    } else {
      type_;
    }
  )
  |> Hashtbl.add(scope.types, name);

let define = (id: Node.Raw.t(string), type_: Type.Raw.strong_t, scope: t) =>
  _safe_define(id, Type.Raw.Strong(type_), scope);

let define_weak =
    (~trait=Type.Trait.Unknown, id: Node.Raw.t(string), scope: t) =>
  _safe_define(id, Type.Raw.Weak(ref(Ok(`Abstract(trait)))), scope);
