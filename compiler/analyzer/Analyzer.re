open Core;

let rec analyze = (~global_scope=Hashtbl.create(24)) =>
  Module.analyze(Scope.create(global_scope));