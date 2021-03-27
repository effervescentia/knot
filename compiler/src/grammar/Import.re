open Kore;

let _from_namespace = imports =>
  Keyword.from
  >> M.string
  >|= Block.value
  >|= Reference.Namespace.of_string
  >|= (namespace => (namespace, imports));

let main =
  M.identifier
  >|= Tuple.split2(Block.value % AST.of_public, Block.cursor)
  >|= AST.of_main
  >|= (x => [x])
  >>= _from_namespace;

let parser =
  Keyword.import >> choice([main]) >|= AST.of_import |> M.terminated;
