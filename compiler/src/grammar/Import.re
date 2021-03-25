open Kore;

let parser =
  Keyword.import
  >> M.identifier
  >|= Block.value
  >>= (
    name =>
      Keyword.from
      >> M.string
      >|= Block.value
      >|= Reference.Namespace.of_string
      >|= (id => (id, name))
      >|= AST.of_import
  )
  |> M.terminated;
