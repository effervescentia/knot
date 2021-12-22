open Infix;
open Reference;

type t = {
  id: int,
  namespace: Namespace.t,
  range: Range.t,
  parent: option(t),
  types: Hashtbl.t(string, Type.Raw.t),
  mutable next_weak_id: int,
  mutable children: list(t),
  /* error reporting callback */
  report: Error.compile_err => unit,
};

/* static */

let create =
    (
      namespace: Namespace.t,
      report: Error.compile_err => unit,
      ~parent: option(t)=?,
      range: Range.t,
    )
    : t => {
  id: parent |> Option.map(({id}) => id + 1) |?: 0,
  namespace,
  range,
  parent,
  types: Hashtbl.create(0),
  next_weak_id: 0,
  children: [],
  report,
};

/* methods */
