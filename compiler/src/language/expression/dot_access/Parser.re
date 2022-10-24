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
            (
              switch (Node.get_type(expr)) {
              | `Struct(props) => props |> List.assoc_opt(fst(prop))
              | `Module(entries) =>
                entries
                |> List.find_map(
                     fun
                     | (name, Type.Container.Value(type_))
                         when name == fst(prop) =>
                       Some(type_)
                     | _ => None,
                   )
              | _ => None
              }
            )
            |?: Type.Raw.(`Unknown),
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};
