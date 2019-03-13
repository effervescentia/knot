include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let resolve_iff = (promise, x) => snd(promise) := Resolved(x);
