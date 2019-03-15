open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, (value, promise)) =>
  (
    switch (value) {
    | "string" => resolved(String_t)
    | "number" => resolved(Number_t)
    | "boolean" => resolved(Number_t)
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | Some(_) as res => res
      | None => raise(InvalidTypeReference)
      }
    }
  )
  |::> promise;
