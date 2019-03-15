include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let resolved = x => ref(Resolved(x));
let synthetic = x => ref(Synthetic(x));
let any = Generic_t(None);

let typeof =
  fun
  | Resolved(t)
  | Synthetic(t) => t
  | _ => raise(InvalidTypeReference);

let t_of = x => typeof((snd(x))^ ^);

let resolve_ref = x => Some(resolved(x));

let allows_type = (x, target) =>
  switch (x, target) {
  | (None, Number_t | String_t | Boolean_t | Nil_t) => true
  | _ => false
  };

let (|::>) = (x, promise) =>
  switch (x) {
  | Some(typ) =>
    promise := typ;
    true;
  | None => raise(InvalidTypeReference)
  };

let (=:=) = ((_, x), y) => x^ := Resolved(y);
let (=.=) = ((_, x), y) => x^ := Synthetic(y);
