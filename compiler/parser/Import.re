open Core;

let stmt =
  M.import
  >> M.identifier
  >>= (
    main =>
      M.from >> M.string ==> (s => Import(s, [no_ctx(MainExport(main))]))
  )
  |> M.terminated;
