include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let declared = x => ref(Declared(x));
let inferred = x => ref(Inferred(x));
let any = Generic_t(None);

let t_ref = x => (snd(x))^;
let t_of = x =>
  (t_ref(x))^
  |> (
    fun
    | Declared(t)
    | Inferred(t) => t
    | _ => raise(InvalidTypeReference)
  );

let is_analyzed = promise =>
  switch (promise^) {
  | Declared(_)
  | Inferred(_) => true
  | _ => false
  };

let is_declared = promise =>
  switch (promise^) {
  | Declared(_) => true
  | _ => false
  };

let extract_ref = x => {
  let x_ref = t_ref(x);

  if (is_analyzed(x_ref)) {
    x_ref;
  } else {
    raise(InvalidTypeReference);
  };
};

let allows_type = (x, target) =>
  switch (x, target) {
  | (None, Number_t | String_t | Boolean_t | Nil_t) => true
  | _ => false
  };

let (|:>) = (x, promise) => promise := x;

let (=.=) = ((_, x), y) => x^ := Inferred(y);
