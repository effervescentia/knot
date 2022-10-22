open Knot.Kore;
open Parse.Onyx;

module Matchers = Grammar.Matchers;
module Symbol = Grammar.Symbol;

let parse = {
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

let pp:
  Fmt.t(AST.raw_expression_t) =>
  Fmt.t((AST.expression_t, AST.untyped_t(string))) =
  (pp_expression, ppf, ((expr, _), (prop, _))) =>
    Fmt.pf(ppf, "%a.%s", pp_expression, expr, prop);
