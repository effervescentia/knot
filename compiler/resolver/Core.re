include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let resolve_iff = (promise, x) => snd(promise) := ref(Resolved(x));

let any_cast = () => Synthetic(Any_t, []);
let any_cast2 = promise => promise := ref(Synthetic(Any_t, []));

let (|::>) = (x, promise) =>
  switch (x) {
  | Some(typ) =>
    promise := typ;
    true;
  | None =>
    promise := ref(any_cast());
    false;
  };
