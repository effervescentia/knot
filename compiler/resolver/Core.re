include Knot.Core;
include Exception;

module NestedHashtbl = Knot.NestedHashtbl;

let declared = x => ref(Declared(x));
let inferred = x => ref(Inferred(x));
let any = Generic_t(None);

let get_t =
  fun
  | Declared(t)
  | Inferred(t) => t
  | _ => raise(InvalidTypeReference);
let t_ref = x => (snd(x))^;
let t_of = x => (t_ref(x))^ |> get_t;

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

let rec (=??) = (x, y) =>
  switch (x, y) {
  /* both of same type */
  | _ when x == y => true

  /* either is any typed */
  | (_, Generic_t(None))
  | (Generic_t(None), _) => true

  /* callable types are comparable */
  | (
      Generic_t(Some(Callable_t(lhs_args, lhs_ret))) |
      Function_t(lhs_args, lhs_ret),
      Generic_t(Some(Callable_t(rhs_args, rhs_ret))) |
      Function_t(rhs_args, rhs_ret),
    ) =>
    List.length(lhs_args) == List.length(rhs_args)
    && get_t(lhs_ret^)
    =?? get_t(rhs_ret^)
    && List.for_all2(
         (lhs, rhs) => get_t(lhs^) =?? get_t(rhs^),
         lhs_args,
         rhs_args,
       )

  | _ => false
  };
