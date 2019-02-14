open Core;

let analyze = (~global_scope=Hashtbl.create(24)) =>
  fun
  | Some(ast) => Some(Module.analyze(Scope.create(global_scope), ast))
  | None => None;