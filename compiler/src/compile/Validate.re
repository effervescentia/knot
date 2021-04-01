/**
 Static analysis validators for compiler state.
 */
open Kore;
open Reference;

/**
 validate that an import graph does not contain cyclic imports
 */
let no_import_cycles = (~report=throw_all, graph: ImportGraph.t) =>
  graph
  |> ImportGraph.find_cycles
  |> (List.map(Namespace.to_string) |> List.map)
  |> (
    fun
    | [] => ()
    | cycles => report(cycles |> List.map(cycle => ImportCycle(cycle)))
  );

/**
 validate that an import graph does not contain unresolved modules
 */
let no_unresolved_modules = (~report=throw_all, graph: ImportGraph.t) =>
  graph
  |> ImportGraph.find_missing
  |> List.map(Namespace.to_string)
  |> (
    fun
    | [] => ()
    | missing =>
      throw_all(missing |> List.map(path => UnresolvedModule(path)))
  );
