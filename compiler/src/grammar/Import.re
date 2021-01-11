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
      % (id => (id, name) |> AST.of_import)
  )
  |> M.terminated;
