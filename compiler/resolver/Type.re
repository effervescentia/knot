open Core;
open NestedHashtbl;

let resolve = (symbol_tbl, promise) =>
  (
    switch (fst(promise)) {
    | "string" => Some(ref(Resolved(String_t)))
    | "number" => Some(ref(Resolved(Number_t)))
    | "boolean" => Some(ref(Resolved(Number_t)))
    | type_ =>
      switch (symbol_tbl.find(type_)) {
      | None => raise(InvalidTypeReference)
      | Some(typ) => Some(ref(Resolved(typ)))
      }
    }
  )
  |::> snd(promise);
