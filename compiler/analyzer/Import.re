open Core;
open Scope;

let add_to_scope = (scope, module_, name) =>
  scope.add(name, {type_: Module, locality: External(module_)});

let rec analyze = (scope, module_) =>
  fun
  | MainExport(name) => add_to_scope(scope, module_, name)
  | ModuleExport(name) => add_to_scope(scope, module_, name)
  | NamedExport(name, new_name) =>
    (
      switch (new_name) {
      | Some(s) => s
      | None => name
      }
    )
    |> add_to_scope(scope, module_);