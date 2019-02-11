open Core;

let rec analyze = (~scope=Hashtbl.create(24)) => Module.analyze(scope);