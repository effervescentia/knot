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

      Analyzer.analyze(~scope, loaded_module)
      |> (
        fun
        | Some(ast) =>
          switch ((KnotResolve.Core.opt_type_ref(ast))^) {
          | Declared(Module_t(deps, _, _)) => (deps, Some(ast))
          | _ => raise(InvalidProgram(target))
          }
        | None => raise(InvalidProgram(target))
      );
    }
  );

let link_defn =
  KnotAnalyze.Scope.(
    (
      global_scope,
      {target, absolute_path, relative_path, pretty_path},
      loaded_module,
    ) =>
      Analyzer.analyze_defn(global_scope, loaded_module)
      |> (
        fun
        | Some(type_) => type_
        | None => raise(InvalidProgram(target))
      )
  );
