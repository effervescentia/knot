include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;
module V = Validators;

let resolved = x => Some(ref(Resolved(x)));
let synthetic = () => ref(Synthetic([]));

let (|::>) = (x, promise) =>
  switch (x) {
  | Some(typ) =>
    promise := typ;
    true;
  | None =>
    promise := synthetic();
    false;
  };

let (=?>) = (x, y) =>
  switch (x, y) {
  | (_, _) when x == y => true
  | (Any_t, _)
  | (_, Any_t) => true
  | _ => false
  };
