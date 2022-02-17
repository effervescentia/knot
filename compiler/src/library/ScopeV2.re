open Infix;

module Identifier = Reference.Identifier;
module Namespace = Reference.Namespace;

type t = {
  id: int,
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  mutable children: list(t),
  types: Hashtbl.t(Identifier.t, TypeV2.Raw.t),
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
  children: [],
  types: Hashtbl.create(0),
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
