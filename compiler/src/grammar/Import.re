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

let parser = (scope: Scope.t) =>
  Keyword.import
  >> (choice([main_import, named_import]) |> M.comma_sep)
  >|= List.flatten
  << Keyword.from
  >>= namespace
  >@= (
    ((namespace, imports)) =>
      imports
      |> List.iter(
           AST.(
             fun
             | Main((id, _)) =>
               scope |> Scope.import(namespace, Public("main"), Some(id))
             | Named((id, _), None) =>
               scope |> Scope.import(namespace, id, None)
             | Named((id, _), Some((label, _))) =>
               scope |> Scope.import(namespace, id, Some(label))
           ),
         )
  )
  >|= AST.of_import
  |> M.terminated;
