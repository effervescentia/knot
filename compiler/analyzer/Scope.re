open Core;

type t('a, 'b, 'c) = {
  is_complete: unit => bool,
  resolve: 'c => unit,
  add: ('a, 'b) => unit,
};

let create = symbol_tbl => {
  let resolver = Resolver.create();

  {
    is_complete: resolver.is_complete,
    resolve: resolver.resolve,
    add: (key, value) => Hashtbl.add(symbol_tbl, key, value),
  };
};