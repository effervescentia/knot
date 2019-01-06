open Core;

let stmt =
  M.import
  >> M.identifier
  >>= (
    main =>
      M.from >> M.string >>= (s => return(Import(s, [MainExport(main)])))
  )
  |> M.terminated;