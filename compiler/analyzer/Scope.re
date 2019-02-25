open Core;

type t = {
  is_complete: unit => bool,
  resolve: resolve_target => unit,
  pending: unit => list(resolve_target),
  nest: (~label: string=?, ~size: int=?, unit) => t,
};

let rec create =
        (
          ~label="anonymous",
          ~resolver=Resolver.create(),
          ~symbol_tbl=NestedHashtbl.create(~label, 24),
          ~module_tbl=Hashtbl.create(24),
          (),
        ) => {
  let rec scope = {
    is_complete: resolver.is_complete,
    resolve: resolver.resolve(symbol_tbl),
    pending: resolver.pending,
    nest: (~label="anonymous", ~size=8, ()) =>
      create(
        ~resolver,
        ~symbol_tbl=symbol_tbl.nest(~label, ~size, ()),
        ~module_tbl,
        (),
      ),
  };

  scope;
};
