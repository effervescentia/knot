open Kore;

let namespace = imports =>
  M.string
  >|= Block.value
  >|= Reference.Namespace.of_string
  >|= (namespace => (namespace, imports));

let main_import =
  M.identifier
  >|= Tuple.split2(Block.value % AST.of_public, Block.cursor)
  >|= AST.of_main
  >|= (x => [x]);

let named_import =
  Identifier.parser
  >>= (
    id => Keyword.as_ >> Identifier.parser >|= (label => (id, Some(label)))
  )
  <|> (Identifier.parser >|= (id => (id, None)))
  |> M.comma_sep
  |> M.between(Symbol.open_closure, Symbol.close_closure)
  >|= Block.value
  >|= List.map(AST.of_named);

let parser =
  Keyword.import
  >> (choice([main_import, named_import]) |> M.comma_sep)
  >|= List.flatten
  << Keyword.from
  >>= namespace
  >|= AST.of_import
  |> M.terminated;
