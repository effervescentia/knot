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

    let pretty_path = Config.module_name(target);
    let absolute_path = path_resolver(target);
    let relative_path = Config.source_path(absolute_path);

    Log.info("%s  %s (%s)", Emoji.link, pretty_path, relative_path);

    let loaded = Loader.load(absolute_path);

    Log.info(
      "%s  %s (%s)",
      Emoji.left_pointing_magnifying_glass,
      pretty_path,
      relative_path,
    );

    Analyzer.analyze(~scope, (), loaded)
    |> (
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
          | _ => Log.error("%s", "...but it was unresolved :(")
          };
        }
      | None => raise(InvalidProgram(target))
    );
  };
