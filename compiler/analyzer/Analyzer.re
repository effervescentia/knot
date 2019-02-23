open Core;

let analyze =
  fun
  | Some(ast) => {
      let scope = Scope.create(~label="module", ());
      let a_ast = Module.analyze(scope, ast);

      if (scope.is_complete()) {
        Some(a_ast);
      } else {
        scope.pending()
        |> List.iter(Debug.print_resolve_target % print_endline);

        None;
      };
    }
  | None => None;
