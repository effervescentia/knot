include Knot.Core;
include Exception;
include Context;

module NestedHashtbl = Knot.NestedHashtbl;

let (=<<) = (x, y) => {
  y(x);

  x;
};

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
    && fst(lhs_ret^)
    =?? fst(rhs_ret^)
    && List.for_all2(
         (lhs, rhs) => fst(lhs^) =?? fst(rhs^),
         lhs_args,
         rhs_args,
       )

  | _ => false
  };
