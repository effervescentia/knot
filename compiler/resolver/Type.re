open Core;

let resolve_mixin = (symbol_tbl, sidecar_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string"
    | "number"
    | "boolean" => throw(InvalidMixinReference)

    | type_ =>
      Log.info("resolving mixin: %s", value);
      switch (NestedHashtbl.find(symbol_tbl, type_)) {
      | Some(State_t(_, props) as res) =>
        Hashtbl.to_seq(props) |> Hashtbl.add_seq(sidecar_tbl);

        res;
      | Some(Style_t(_) as res) => res
      | None => throw(InvalidTypeReference)
      | _ => throw(InvalidMixinReference)
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

    | _ => throw(InvalidTypeReference)
    }
  )
  <:= promise;
