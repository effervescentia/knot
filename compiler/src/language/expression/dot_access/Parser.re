open Knot.Kore;
open Parse.Kore;
open AST;

let parse = {
  let rec loop = expr =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      prop =>
        loop(
          Node.typed(
            (expr, prop) |> Raw.of_dot_access,
            (),
            Node.get_range(prop),
          ),
        )
    )
    |> option(expr);

  loop;
};
