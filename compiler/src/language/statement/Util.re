open Kore;

let fold = (~variable, ~effect) =>
  AST.Statement.(
    fun
    | Variable(name, expression) => (name, expression) |> variable
    | Effect(expression) => expression |> effect
  );
