open Kore;

let constant =
  Keyword.const
  >> Operator.assign(
       M.identifier >|= Block.value,
       Expression.parser >|= AST.of_const,
     )
  |> M.terminated;

let parser = choice([constant]) >|= AST.of_decl;
