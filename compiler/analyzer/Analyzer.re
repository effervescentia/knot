open Core;

let analyze = (~scope=Scope.create(~label="module", ()), ()) =>
  fun
  | Some(ast) => {
      let a_ast = Module.analyze(scope, ast);

      if (!scope.is_complete()) {
        scope.pending()
        |> List.iter(Debug.print_resolve_target % print_endline);
      };

      Some(a_ast);
    }
  | None => None;

let rec analyze_dependencies =
  fun
  | A_Module(xs) =>
    List.fold_left(
      (acc, x) =>
        switch (abandon_ctx(x)) {
        | A_Import(module_, imports) => [module_, ...acc]
        | _ => acc
        },
      [],
      xs,
    )
    |> List.map(String.trim)
    |> List.filter(x => x != "");
