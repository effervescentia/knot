open Core;

type t('a, 'b) = {
  label: string,
  add: ('a, 'b) => unit,
  nest: (~label: string=?, ~size: int=?, unit) => t('a, 'b),
  find: 'a => option('b),
};

let nested_name = s =>
  fun
  | Some({label}) => Printf.sprintf("%s.%s", label, s)
  | None => s;

let rec create = (~label="anonymous", ~parent=?, size) => {
  let symbol_tbl = Hashtbl.create(size);
  let real_label = nested_name(label, parent);

  let rec scope = {
    label: real_label,
    add: (key, value) => {
      Printf.sprintf("Adding '%s' to scope '%s'.", key, real_label)
      |> print_endline;

      Hashtbl.add(symbol_tbl, key, value);
    },
    nest: (~label="anonymous", ~size=8, ()) =>
      create(~label, ~parent=scope, size),
    find: key => {
      Printf.sprintf("Looking for '%s' in scope '%s'", key, real_label)
      |> print_endline;

      switch (Some(Hashtbl.find(symbol_tbl, key))) {
      | res =>
        Printf.sprintf("found in scope '%s'!", real_label) |> print_endline;

        res;
      | exception Not_found =>
        switch (parent) {
        | Some(tbl) => tbl.find(key)
        | None =>
          print_endline("not found...");
          None;
        }
      };
    },
  };

  scope;
};
