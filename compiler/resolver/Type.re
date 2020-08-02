open Globals;

let resolve_mixin = (symbol_tbl, sidecar_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string"
    | "number"
    | "boolean" => throw_semantic(UnsupportedMixinType(value))

    | _ =>
      Log.info("resolving mixin: %s", value);
      switch (NestedHashtbl.find(symbol_tbl, value)) {
      | Some(State_t(_, props) as res) =>
        Hashtbl.to_seq(props) |> Hashtbl.add_seq(sidecar_tbl);

        res;
      | Some(Style_t(_) as res) => res
      | Some(_) => throw_semantic(UnsupportedMixinType(value))
      | None => throw_semantic(TypeDoesNotExist(value))
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

    | _ => throw_semantic(UnsupportedTypeReference(value))
    }
  )
  <:= promise;
