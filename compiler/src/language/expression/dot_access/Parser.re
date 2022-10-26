open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let dot_access = {
  let rec loop = expr =>
    Symbol.period
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
