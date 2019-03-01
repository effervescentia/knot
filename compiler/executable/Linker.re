open Kore;
open KnotAnalyze.Scope;

exception InvalidProgram(string);

let link = (path_resolver, global_scope, notify, target) =>
  if (!Hashtbl.mem(global_scope.module_tbl, target)) {
    let scope =
      global_scope.nest(
        ~label=Printf.sprintf("module(%s)", target),
        ~size=24,
        (),
      );

    let absolute_path = path_resolver(target);

    Printf.sprintf("linking module: %s (%s)", target, absolute_path)
    |> print_endline;

    Loader.load(absolute_path)
    |> Analyzer.analyze(~scope, ())
    % (
      fun
      | Some(ast) => {
          Hashtbl.add(
            global_scope.module_tbl,
            target,
            Loaded(absolute_path, ast),
          );

          switch (ast^) {
          | Resolved(_, Module_t(deps, _, _)) => List.iter(notify, deps)
          | Pending(ast, _) =>
            Analyzer.analyze_dependencies(ast) |> List.iter(notify)
          | _ => print_endline("...but it was unresolved :(")
          };
        }
      | None => raise(InvalidProgram(target))
    );
  };
