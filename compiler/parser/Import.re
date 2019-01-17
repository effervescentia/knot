open Core;

let stmt = input =>
  (
    M.import
    >> M.identifier
    >>= (
      main => M.from >> M.string ==> (s => Import(s, [MainExport(main)]))
    )
    |> M.terminated
  )(
    input,
  );
