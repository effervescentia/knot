open Core;

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

let analyze = (~scope=Scope.create(~label="module", ()), ()) =>
  fun
  | Some(ast) => Some(Module.analyze(scope, ast))
  | None => None;
