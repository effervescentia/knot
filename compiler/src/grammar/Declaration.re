open Kore;

let constant =
  Keyword.const
  >> Operator.assign(
       M.identifier
       >|= (
         id => {
           let value = id |> Block.value;

           (
             String.starts_with(Constants.private_prefix, value)
               ? value
                 |> String.drop_prefix(Constants.private_prefix)
                 |> AST.of_private
               : AST.of_public(value),
             id |> Block.cursor,
           );
         }
       ),
       Expression.parser >|= AST.of_const,
     )
  |> M.terminated;

let parser = choice([constant]) >|= AST.of_decl;
