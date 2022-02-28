open Infix;

module Identifier = Reference.Identifier;
module Namespace = Reference.Namespace;

type t = {
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  mutable children: list(t),
  types: Hashtbl.t(Identifier.t, TypeV2.t),
  /* error reporting callback */
  report: Error.compile_err => unit,
};

/* static */

let rec _with_root = (f: t => 'a, scope: t): 'a =>
  switch (scope.parent) {
  | Some(parent) => _with_root(f, parent)
  | None => f(scope)
  };

let create =
    (
      namespace: Namespace.t,
      report: Error.compile_err => unit,
      ~parent: option(t)=?,
      range: Range.t,
    )
    : t => {
  namespace,
  range,
  parent,
  children: [],
  types: Hashtbl.create(0),
  report,
};

/* methods */

/**
 create a new child scope from a parent scope and register them with each other
 */
let create_child = (range: Range.t, parent: t): t => {
  let child = create(~parent, parent.namespace, parent.report, range);

  parent.children = parent.children @ [child];

  child;
};

/**
 find a type in this or any parent scope
 */
let rec lookup = (id: Identifier.t, scope: t): option(TypeV2.t) => {
  switch (scope.parent, Hashtbl.find_opt(scope.types, id)) {
  | (_, Some(type_)) => Some(type_)

  | (Some(parent), _) => parent |> lookup(id)

  | _ => None
  };
};

/**
 define a new type in this scope
 */
let define =
    (id: Identifier.t, type_: TypeV2.t, scope: t): option(TypeV2.error_t) => {
  let result =
    scope |> lookup(id) |> Option.map(_ => TypeV2.DuplicateIdentifier(id));

  Hashtbl.add(scope.types, id, type_);

  result;
};

let report_type_err = (scope: t, range: Range.t, err: TypeV2.error_t) =>
  scope.report(ParseError(TypeErrorV2(err), scope.namespace, range));
