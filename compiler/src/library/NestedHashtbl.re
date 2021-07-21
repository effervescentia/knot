open Infix;

type t('a, 'b) = {
  parent: option(t('a, 'b)),
  table: Hashtbl.t('a, 'b),
};

/* static */

let create = (~parent: option(t('a, 'b))=?, size: int) => {
  parent,
  table: Hashtbl.create(size),
};

let from_seq = (seq: Seq.t(('a, 'b))): t('a, 'b) => {
  parent: None,
  table: Hashtbl.of_seq(seq),
};

/* methods */

let child = (parent: t('a, 'b)): t('a, 'b) => create(~parent, 0);

let rec find = (key: 'a, {table, parent}: t('a, 'b)): option('b) =>
  switch (Hashtbl.find_opt(table, key), parent) {
  | (Some(res), _) => Some(res)
  | (None, Some(tbl)) => tbl |> find(key)
  | _ => None
  };

let set = (key: 'a, value: 'b, {table}: t('a, 'b)): unit =>
  Hashtbl.replace(table, key, value);
