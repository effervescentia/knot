open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => Some(resolved(String_t))
    | "number" => Some(resolved(Number_t))
    | "boolean" => Some(resolved(Boolean_t))
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | Some(_) as res => res
      | None => raise(InvalidTypeReference)
      }
    }
  )
  |::> promise;
