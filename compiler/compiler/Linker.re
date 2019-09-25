open Core;

module Analyzer = KnotAnalyze.Analyzer;

let link =
  KnotAnalyze.Scope.(
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path},
      loaded_module,
    ) => {
      /* create a module-level scope */
      let scope =
        global_scope.nest(
          ~label=Printf.sprintf("module(%s)", target),
          ~size=24,
          (),
        );

      Analyzer.analyze(~scope, loaded_module)
      |!> CompilationError(AnalysisFailed(target));
    }
  );

let link_defn = Analyzer.analyze_defn;
