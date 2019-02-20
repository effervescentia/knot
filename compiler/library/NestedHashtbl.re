open Core;

type t('a, 'b) = {
  add: ('a, 'b) => unit,
  nest: (~size: int=?, unit) => t('a, 'b),
};

let rec create = (~parent=?, symbol_tbl) => {
  let nested = ref([]);

  let rec scope = {
    add: (key, value) => Hashtbl.add(symbol_tbl, key, value),
    nest: (~size=8, ()) => {
      let nested_scope = create(~parent=scope, Hashtbl.create(size));

      nested := [nested_scope, ...nested^];

      nested_scope;
    },
  };

  scope;
};
