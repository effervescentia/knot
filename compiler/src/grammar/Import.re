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
      >|= (
        id => (
          String.starts_with(Constants.root_dir, id)
            ? AST.of_internal(String.drop_prefix(Constants.root_dir, id))
            : AST.of_external(id),
          name,
        )
      )
      >|= AST.of_import
  )
  |> M.terminated;
