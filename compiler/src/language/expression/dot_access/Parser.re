open Knot.Kore;
open Parse.Kore;
open AST;

let parse = {
  let rec loop = object_ =>
    Matchers.period
    >> Matchers.identifier
    >>= (
      property =>
        Node.raw(
          (object_, property) |> Raw.of_dot_access,
          Node.get_range(property),
        )
        |> loop
    )
    |> option(object_);

  loop;
};
