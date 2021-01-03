open Kore;

let constant =
  Keyword.const
  >> M.binary_op(
       M.identifier >|= fst,
       Operator.assign,
       Expression.parser >|= AST.of_const,
     )
  |> M.terminated;

let parser = choice([constant]) >|= AST.of_decl;
