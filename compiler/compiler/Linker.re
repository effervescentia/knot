open Core;

module Analyzer = KnotAnalyze.Analyzer;

let link =
  KnotAnalyze.Scope.(
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path},
      loaded_module,
    ) => {
      let scope =
        global_scope.nest(
          ~label=Printf.sprintf("module(%s)", target),
          ~size=24,
          (),
        );

      Analyzer.analyze(~scope, (), loaded_module)
      |> (
        fun
        | Some(ast) =>
          switch ((snd(ast))^) {
          | Resolved(Module_t(deps, _, _)) => (deps, Some(ast))
          | Pending(_) => (Analyzer.analyze_dependencies(fst(ast)), None)
          | _ => raise(InvalidProgram(target))
          }
        | None => raise(InvalidProgram(target))
      );
    }
  );
