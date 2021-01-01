open Kore;

let parser =
  Keyword.import
  >> M.identifier
  >|= fst
  >>= (
    name =>
      Keyword.from
      >> Primitive.string
      >|= fst
      >>= (id => (id, name) |> AST.of_import |> return)
  )
  |> M.terminated;
