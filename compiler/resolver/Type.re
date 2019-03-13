open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  (
    switch (fst(promise)) {
    | "string" => Some(String_t)
    | "number" => Some(Number_t)
    | "boolean" => Some(Number_t)
    | _ => raise(InvalidTypeReference)
    }
  )
  |%> resolve_iff(promise);
