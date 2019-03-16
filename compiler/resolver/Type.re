open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => Some(declared(String_t))
    | "number" => Some(declared(Number_t))
    | "boolean" => Some(declared(Boolean_t))
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | Some(_) as res => res
      | None => raise(InvalidTypeReference)
      }
    }
  )
  |::> promise;
