open Kore;

let fold = (~variable, ~effect) =>
  AST.Expression.(
    fun
    | Variable(name, expression) => (name, expression) |> variable
    | Effect(expression) => expression |> effect
  );
