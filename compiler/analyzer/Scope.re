open Core;

type t = {
  is_resolved: unit => bool,
  resolve: resolve_target => unit,
  pending: unit => list(resolve_target),
  nest: (~label: string=?, ~size: int=?, unit) => t,
  module_tbl: Hashtbl.t(string, linked_module),
};

let rec create =
        (
          ~label="anonymous",
          ~symbol_tbl=NestedHashtbl.create(~label, 24),
          ~module_tbl=Hashtbl.create(24),
          ~resolver=?,
          (),
        ) => {
  let real_resolver =
    switch (resolver) {
    | Some(r) => r
    | None => Resolver.create(module_tbl)
    };
  {
    module_tbl,
    is_resolved: real_resolver.is_complete,
    resolve: real_resolver.resolve(symbol_tbl),
    pending: real_resolver.pending,
    nest: (~label="anonymous", ~size=8, ()) =>
      create(
        ~resolver=real_resolver,
        ~symbol_tbl=symbol_tbl.nest(~label, ~size, ()),
        ~module_tbl,
        (),
      ),
  };
};
