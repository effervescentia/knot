open Operators;
open Invariant;

type t('a, 'b) = {
  boundary: bool,
  label: string,
  parent: option(t('a, 'b)),
  tbl: Hashtbl.t('a, 'b),
};

let _anon_scope = "anonymous";

let _nested_name = s =>
  fun
  | Some({label}) => Printf.sprintf("%s.%s", label, s)
  | None => s;

/** create a new table */
let create = (~label=_anon_scope, ~parent=?, ~boundary=false, size) => {
  boundary,
  parent,
  label: _nested_name(label, parent),
  tbl: Hashtbl.create(size),
};

/** add an entry to the table */
let add = ({tbl}) => Hashtbl.add(tbl);

let rec expect = ({boundary, tbl, parent}, key, value) =>
  boundary
    ? Hashtbl.add(tbl, key, value)
    : (
      switch (parent) {
      | Some(p_tbl) => expect(p_tbl, key, value)
      | _ => invariant(BoundaryScopeMissing)
      }
    );

/** create a child table */
let nest = (~label=_anon_scope, ~size=8, tbl) =>
  create(~label, ~parent=tbl, size);

/** find a value without checking ancestor tables */
let find_local = ({tbl}, key) =>
  try(Some(Hashtbl.find(tbl, key))) {
  | Not_found => None
  };

/** find a value and fallback to checking ancestor tables */;
let rec find = (tbl, key) =>
  switch (find_local(tbl, key)) {
  | None => tbl.parent |?> (p_tbl => find(p_tbl, key))
  | res => res
  };
