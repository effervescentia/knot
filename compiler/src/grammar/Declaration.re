open Kore;

let constant_decl =
  Keyword.const >> return(("", AST.nil |> AST.of_prim |> AST.of_const));

let constant_decl = Keyword.const >> M.identifier >>= (id => return());

let parser = choice([constant_decl]) >|= AST.of_decl;
