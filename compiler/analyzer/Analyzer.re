open Core;

let analyze = (~scope=Scope.create(~label="module", ()), ()) =>
  fun
  | Some(ast) => {
      Module.analyze(scope, ast);

      if (!scope.is_complete()) {
        scope.pending()
        |> List.iter(Debug.print_resolve_target % print_endline);
      };

      Some(ast);
    }
  | None => None;

let rec analyze_dependencies =
  fun
  | Module(xs) =>
    List.fold_left(
      acc =>
        fun
        | Import(module_, imports) => [module_, ...acc]
        | _ => acc,
      [],
      xs,
    )
    |> List.map(String.trim)
    |> List.filter(x => x != "");
