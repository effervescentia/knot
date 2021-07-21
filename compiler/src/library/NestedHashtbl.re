open Infix;

type t('a, 'b) = {
  parent: option(t('a, 'b)),
  table: Hashtbl.t('a, 'b),
};

let create = (~parent: option(t('a, 'b))=?, ()) => {
  parent,
  table: Hashtbl.create(0),
};

let child = (parent: t('a, 'b)) => create(~parent, ());

let rec find = (key: 'a, {table, parent}: t('a, 'b)) =>
  switch (Hashtbl.find_opt(table, key), parent) {
  | (Some(res), _) => Some(res)
  | (None, Some(tbl)) => tbl |> find(key)
  | _ => None
  };

let set = (key: 'a, value: 'b, {table}: t('a, 'b)) =>
  Hashtbl.replace(table, key, value);
