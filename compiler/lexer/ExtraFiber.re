open Core;
open Knot.Fiber;

let tuple_satisfy:
  ('a => bool, Opal.LazyStream.t(('a, context_mutator))) =>
  option(
    (('a, context_mutator), Opal.LazyStream.t(('a, context_mutator))),
  ) =
  f => satisfy(((y, _)) => f(y));

let tuple_exactly = x => tuple_satisfy((==)(x));

let tuple_one_of = ls => tuple_satisfy(x => List.mem(x, ls));
