open Core;

let analyze_dependencies =
  fun
  | Module(imports, _) =>
    List.fold_left(
      acc =>
        fun
        | Import(module_, imports) => [module_, ...acc],
      [],
      imports,
    )
    |> List.map(String.trim)
    |> List.filter(x => x != "");

let analyze = (~scope=Scope.create(~label="module", ~boundary=true, ()), x) =>
  x |?> Module.analyze(scope) % some;

let analyze_defn = Definition.analyze;
