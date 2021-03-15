open Kore;

let constant =
  Keyword.const
  >> Operator.assign(
       M.identifier
       >|= Block.value
       >|= (
         x =>
           String.starts_with(Constants.private_prefix, x)
             ? x
               |> String.drop_prefix(Constants.private_prefix)
               |> AST.of_private
             : AST.of_public(x)
       ),
       Expression.parser >|= AST.of_const,
     )
  |> M.terminated;

let parser = choice([constant]) >|= AST.of_decl;
