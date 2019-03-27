open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => String_t
    | "number" => Number_t
    | "boolean" => Boolean_t

    /* TODO: support user-defined types */
    | _ => raise(InvalidTypeReference)
    /* | type_ =>
       switch (symbol_tbl.find(type_)) {
       | Some(res) => res
       | None => raise(InvalidTypeReference)
       } */
    }
  )
  <:= promise;
