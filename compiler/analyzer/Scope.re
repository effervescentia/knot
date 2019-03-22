open Core;

exception InvalidScope;

type t = {
  resolve: resolve_target => unit,
  nest: (~label: string=?, ~size: int=?, unit) => t,
  module_tbl: Hashtbl.t(string, linked_module),
  validate: unit => unit,
};

let rec create =
        (
          ~label="anonymous",
          ~boundary=false,
          ~symbol_tbl=NestedHashtbl.create(~label, ~boundary, 24),
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
    validate: () =>
      symbol_tbl.iter_local((key, typ) =>
        switch (typ^) {
        | Inferred(_) => raise(InvalidScope)
        | _ => ()
        }
      ),
  };
};
