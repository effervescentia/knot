open Kore;

let constant =
  Keyword.const
  >> M.binary_op(
       M.identifier >|= Block.value,
       Operator.assign,
       Expression.parser >|= AST.of_const,
     )
  |> M.terminated;

let parser = choice([constant]) >|= AST.of_decl;
