open Core;
open Resolver;

type t = {
  resolve: resolve_target => unit,
  nest:
    (
      ~label: string=?,
      ~size: int=?,
      ~sidecar: Hashtbl.t(string, member_type)=?,
      unit
    ) =>
    t,
  module_tbl: Hashtbl.t(string, linked_module),
};

let rec create =
        (
          ~label="anonymous",
          ~boundary=false,
          ~symbol_tbl=NestedHashtbl.create(~label, ~boundary, 24),
          ~module_tbl=Hashtbl.create(24),
          ~resolver=?,
          ~sidecar=?,
          (),
        ) => {
  let real_resolver =
    switch (resolver) {
    | Some(r) => r
    | None => Resolver.create(module_tbl)
    };
  {
    module_tbl,
    resolve: real_resolver.resolve({symbol_tbl, sidecar}),
    nest: (~label="anonymous", ~size=8, ~sidecar as nested_sidecar=?, ()) =>
      create(
        ~resolver=real_resolver,
        ~symbol_tbl=NestedHashtbl.nest(~label, ~size, symbol_tbl),
        ~module_tbl,
      )
      |> (
        f =>
          switch (nested_sidecar) {
          | Some(x) => f(~sidecar=x, ())
          | None =>
            switch (sidecar) {
            | Some(x) => f(~sidecar=x, ())
            | None => f()
            }
          }
      ),
  };
};
