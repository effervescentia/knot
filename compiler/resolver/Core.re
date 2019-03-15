include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let resolved = x => Some(ref(Resolved(x)));

let typeof =
  fun
  | Resolved(t)
  | Synthetic(t) => t
  | _ => raise(InvalidTypeReference);

let (|::>) = (x, promise) =>
  switch (x) {
  | Some(typ) =>
    promise := typ;
    true;
  | None => raise(InvalidTypeReference)
  };
