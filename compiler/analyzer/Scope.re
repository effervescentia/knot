open Core;

type t = {
  resolve: resolve_target => unit,
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
    resolve: real_resolver.resolve(symbol_tbl),
    nest: (~label="anonymous", ~size=8, ()) =>
      create(
        ~resolver=real_resolver,
        ~symbol_tbl=symbol_tbl.nest(~label, ~size, ()),
        ~module_tbl,
        (),
      ),
  };
};
