open Infix;
open Reference;

/**
 this should never occur in practice, but not sure how to ensure that in the code
 */
exception WeakTypeNotFound;

type t = {
  id: int,
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  types: Hashtbl.t(Identifier.t, Type.Raw.t),
  weak_types: Hashtbl.t(int, result(Type.Raw.strong_t, Type.Raw.error_t)),
  mutable children: list(t),
  /* error reporting callback */
  report: Error.compile_err => unit,
};

/* returned by [resolve_type] */
type resolved_t =
  | Strong(Type.Raw.strong_t)
  | Weak(t, int, Type.Raw.strong_t)
  | Invalid(Type.Raw.error_t);

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

let report_raw = (err, range: Range.t, scope: t) => {
  scope.report(ParseError(TypeError(err), scope.namespace, range));

  Type.Raw.Invalid(err);
};

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
        : option(result(Type.Raw.strong_t, Type.Raw.error_t)) =>
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
 lookup a weak type by its unique identifiers
 */
let rec find_weak =
        (scope_id: int, weak_id: int, scope: t)
        : (t, result(Type.Raw.strong_t, Type.Raw.error_t)) =>
  switch (scope_id === scope.id, scope.parent) {
  | (true, _) => (
      scope,
      try(Hashtbl.find(scope.weak_types, weak_id)) {
      | Not_found => raise(WeakTypeNotFound)
      },
    )
  | (_, Some(parent)) => find_weak(scope_id, weak_id, parent)
  | _ => raise(WeakTypeNotFound)
  };

/**
 resolve a type in the scope to get its current value (accounts for weak type redirection)
 */
let resolve_type = (node: Node.t('a, Type.Raw.t), scope: t): resolved_t =>
  switch (Node.get_type(node)) {
  /* incoming type meets requirements and can be forwarded */
  | Type.Raw.Strong(t) => Strong(t)

  /* weak type needs to be looked up and resolved */
  /* TODO: maybe revert to keeping a ref in the type itself as well as in the lookup to avoid this */
  | Type.Raw.Weak(scope_id, weak_id) =>
    let (weak_scope, weak_type) = scope |> find_weak(scope_id, weak_id);

    switch (weak_type) {
    /* incoming type meets requirements and can be forwarded */
    | Ok(t) => Weak(weak_scope, weak_id, t)

    /* other weak types cannot be narrowed */
    | Error(err) => Invalid(err)
    };

  /* type does not match the strong type */
  | Type.Raw.Invalid(err) => Invalid(err)
  };

/**
 test a type in the scope and narrow it if it can be narrowed
 */
let test_and_narrow =
    (
      validate: Type.Raw.strong_t => bool,
      narrowed: Type.Raw.strong_t,
      node: Node.t('a, Type.Raw.t),
      scope: t,
    ) => {
  let type_ = scope |> resolve_type(node);

  (
    switch (type_) {
    /* incoming type meets requirements and can be forwarded */
    | Strong(t)
    | Weak(_, _, t) when validate(t) => Ok((Node.get_type(node), Some(t)))

    /* type does not match the strong type */
    | Strong(t) =>
      Error(
        Type.Error.TypeMismatch(
          Type.Raw.Strong(narrowed),
          Type.Raw.Strong(t),
        ),
      )

    /* weak generic type can be narrowed */
    | Weak(weak_scope, weak_id, `Generic(_)) =>
      Hashtbl.replace(weak_scope.weak_types, weak_id, Ok(narrowed));

      Ok((Node.get_type(node), Some(narrowed)));

    /* other weak types cannot be narrowed */
    | Weak(weak_scope, weak_id, weak_type) =>
      Error(
        Type.Error.NotNarrowable(
          Type.Raw.Strong(narrowed),
          Type.Raw.Strong(weak_type),
        ),
      )

    | Invalid(err) => Ok((Type.Raw.Invalid(err), None))
    }
  )
  |> (
    fun
    | Ok(result) => result
    | Error(err) => (scope |> report_raw(err, Node.get_range(node)), None)
  );
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

    /* TODO: do this again? */
    /* scope |> loop |> Hashtbl.of_seq; */

    Hashtbl.create(0);
  };
