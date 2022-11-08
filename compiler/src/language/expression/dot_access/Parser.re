open Knot.Kore;
open Parse.Onyx;

module Matchers = Parse.Matchers;

let dot_access = {
  let rec loop = expr =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.typed(
            (expr, prop) |> AST.Raw.of_dot_access,
            (),
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};
