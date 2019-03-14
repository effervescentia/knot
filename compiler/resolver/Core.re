include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let resolve_iff = (promise, x) => snd(promise) := ref(Resolved(x));

let any_cast = () => Synthetic(Any_t, []);
