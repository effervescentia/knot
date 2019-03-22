open Core;

exception BoundaryScopeMissing;

type t('a, 'b) = {
  label: string,
  add: ('a, 'b) => unit,
  expect: ('a, 'b) => unit,
  nest: (~label: string=?, ~size: int=?, unit) => t('a, 'b),
  find: 'a => option('b),
  find_local: 'a => option('b),
  iter_local: (('a, 'b) => unit) => unit,
};

let nested_name = s =>
  fun
  | Some({label}) => Printf.sprintf("%s.%s", label, s)
  | None => s;

let rec create = (~label="anonymous", ~parent=?, ~boundary=false, size) => {
  let symbol_tbl = Hashtbl.create(size);
  let real_label = nested_name(label, parent);

  let rec scope = {
    label: real_label,
    add: (key, value) => Hashtbl.add(symbol_tbl, key, value),
    expect: (key, value) =>
      boundary ?
        Hashtbl.add(symbol_tbl, key, value) :
        (
          switch (parent) {
          | Some(tbl) => tbl.expect(key, value)
          | _ => raise(BoundaryScopeMissing)
          }
        ),
    nest: (~label="anonymous", ~size=8, ()) =>
      create(~label, ~parent=scope, size),
    find_local: key =>
      switch (Some(Hashtbl.find(symbol_tbl, key))) {
      | res => res
      | exception Not_found => None
      },
    find: key =>
      switch (scope.find_local(key)) {
      | None =>
        switch (parent) {
        | Some(tbl) => tbl.find(key)
        | None => None
        }
      | res => res
      },
    iter_local: f => Hashtbl.iter(f, symbol_tbl),
  };

  scope;
};
