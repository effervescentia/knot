include Knot.Core;
include AnnotatedAST;

type member_type =
  | Number
  | String
  | Boolean
  | Function
  | View
  | State
  | Style
  | Module;

type member_locality =
  | External(string)
  | Module;

type scope_member = {
  type_: member_type,
  locality: member_locality,
};

let wrap = x => ref(Pending(x));

let wrap_opt =
  fun
  | Some(x) => Some(wrap(x))
  | None => None;

let wrap_and_trfm_opt = transform =>
  fun
  | Some(x) => Some(transform(x) |> wrap)
  | None => None;

let analyze_list = analyze => List.map(analyze % wrap);