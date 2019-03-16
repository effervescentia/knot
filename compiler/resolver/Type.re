open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => declared(String_t)
    | "number" => declared(Number_t)
    | "boolean" => declared(Boolean_t)
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | Some(res) => res
      | None => raise(InvalidTypeReference)
      }
    }
  )
  |:> promise;
