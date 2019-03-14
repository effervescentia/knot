open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  (
    switch (fst(promise)) {
    | "string" => Some(String_t)
    | "number" => Some(Number_t)
    | "boolean" => Some(Number_t)
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | None => raise(InvalidTypeReference)
      | res => res
      }
    }
  )
  |%> resolve_iff(promise);
