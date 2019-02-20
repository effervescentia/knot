open Core;

type t('a, 'b) = {
  add: ('a, 'b) => unit,
  is_complete: unit => bool,
  resolve: resolve_target => unit,
  nest: (~size: int=?, unit) => t('a, 'b),
};

let rec create = (~resolver=Resolver.create(), ~parent=?, symbol_tbl) => {
  let nested = ref([]);

  let rec scope = {
    add: (key, value) => Hashtbl.add(symbol_tbl, key, value),
    is_complete: resolver.is_complete,
    resolve: resolver.resolve,
    nest: (~size=8, ()) => {
      let nested_scope =
        create(~resolver, ~parent=scope, Hashtbl.create(size));

      nested := [nested_scope, ...nested^];

      nested_scope;
    },
  };

  scope;
};