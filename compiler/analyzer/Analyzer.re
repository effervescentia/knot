open Core;

let analyze = (~global_scope=Hashtbl.create(24)) =>
  fun
  | Some(ast) => {
      let scope = Scope.create(global_scope);
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
