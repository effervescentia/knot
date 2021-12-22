open Infix;
open Reference;

type t = {
  id: int,
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  types: Hashtbl.t(Identifier.t, Type.Raw.t),
  weak_types: Hashtbl.t(int, result(Type.Raw.weak_t, Type.Raw.error_t)),
  mutable children: list(t),
  /* error reporting callback */
  report: Error.compile_err => unit,
};

/* static */

let rec _with_root = (f: t => 'a, scope: t): 'a =>
  switch (scope.parent) {
  | Some(parent) => _with_root(f, parent)
  | None => f(scope)
  };

let _count_scopes =
  _with_root(
    {
      let rec count = scope =>
        List.length(scope.children)
        + (scope.children |> List.map(count) |> List.fold_left((+), 0));

      count;
    },
  );

let create =
    (
      namespace: Namespace.t,
      report: Error.compile_err => unit,
      ~parent: option(t)=?,
      range: Range.t,
    )
    : t => {
  id: parent |> Option.map(_count_scopes) |?: 0,
  namespace,
  range,
  parent,
  types: Hashtbl.create(0),
  weak_types: Hashtbl.create(0),
  children: [],
  report,
};

/* methods */

/**
 create a new child scope from a parent scope and register them with each other
 */
let create_child = (parent: t, range: Range.t): t => {
  let child = create(~parent, parent.namespace, parent.report, range);

  parent.children = parent.children @ [child];

  child;
};

/**
 creates a new weak type that can be uniquely identified by
 a combination of the embedded scope_id and the weak_id values
 */
let next_weak_type = (scope: t): Type.Raw.t => {
  let weak_id = Hashtbl.length(scope.weak_types);
  let weak_type = Type.Raw.Weak(scope.id, weak_id);

  Hashtbl.add(scope.weak_types, weak_id, Ok(`Generic((scope.id, weak_id))));

  weak_type;
};

/**
 check if there are any weak types defined in this or any child scope
 */
let rec is_resolved = (scope: t): bool =>
  Hashtbl.length(scope.weak_types) == 0
  && scope.children
  |> List.for_all(is_resolved);

/**
 check if a type is defined matching the provided identifier
 */
let rec exists = (name: Identifier.t, scope: t): bool =>
  Hashtbl.mem(scope.types, name)
  || (
    switch (scope.parent) {
    | Some(parent) => parent |> exists(name)
    | _ => false
    }
  );

/**
 define a type given an identifier and the type value

 if the type already exists then a DuplicateIdentifier error will be reported
 and an Invalid type containing the error details will be applied instead
 */
let define =
    ((name, range): Node.Raw.t(Identifier.t), type_: Type.Raw.t, scope: t) =>
  (
    if (scope |> exists(name)) {
      let type_err = Type.Error.DuplicateIdentifier(name);

      scope.report(ParseError(TypeError(type_err), scope.namespace, range));

      Type.Raw.Invalid(type_err);
    } else {
      type_;
    }
  )
  |> Hashtbl.add(scope.types, name);

let rec _resolve_raw =
        (scope: t, scope_id: int, weak_id: int)
        : option(result(Type.Raw.weak_t, Type.Raw.error_t)) =>
  if (scope.id == scope_id) {
    Hashtbl.find_opt(scope.weak_types, weak_id);
  } else {
    switch (scope.parent) {
    | Some(parent) => _resolve_raw(parent, scope_id, weak_id)
    | None => None
    };
  };

/**
 lookup a type by its identifier in this and every ancestor scope

 if the type does not exist then a NotFound error will be reported
 and an Invalid type containing the error details will be returned instead
 */
let rec resolve =
        ((name, range) as id: Node.Raw.t(Identifier.t), scope: t)
        : Type.Raw.t =>
  switch (Hashtbl.find_opt(scope.types, name), scope.parent) {
  | (Some(type_), _) => type_
  | (_, Some(parent)) => parent |> resolve(id)
  | _ =>
    let type_err = Type.Error.NotFound(name);

    scope.report(ParseError(TypeError(type_err), scope.namespace, range));

    Invalid(type_err);
  };

/**
 generate finalized type mappings for all registered weak types
 */
let finalize = (scope: t): Hashtbl.t((int, int), Type.t) =>
  if (is_resolved(scope)) {
    Hashtbl.create(0);
  } else {
    let rec loop = s =>
      Seq.append(
        s.weak_types
        |> Hashtbl.to_seq
        |> Seq.map(((weak_id, weak_type)) =>
             Type.((s.id, weak_id), weak_type)
           ),
        s.children |> List.to_seq |> Seq.flat_map(loop),
      );

    /* scope |> loop |> Hashtbl.of_seq; */
    Hashtbl.create(0);
  };
