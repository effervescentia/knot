open Core;
open NestedHashtbl;

let resolve_mixin = (symbol_tbl, sidecar_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string"
    | "number"
    | "boolean" => raise(InvalidMixinReference)

    | type_ =>
      Log.info("resolving mixin: %s", value);
      switch (symbol_tbl.find(type_)) {
      | Some(State_t(_, props) as res) =>
        Hashtbl.to_seq(props) |> Hashtbl.add_seq(sidecar_tbl);

        res;
      | Some(Style_t(_) as res) => res
      | None => raise(InvalidTypeReference)
      | _ => raise(InvalidMixinReference)
      };
    }
  )
  <:= promise;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => String_t
    | "number" => Number_t
    | "boolean" => Boolean_t

    | _ => raise(InvalidTypeReference)
    }
  )
  <:= promise;
