/**
 Static analysis validators for compiler state.
 */
open Kore;
open Reference;

/**
 validate that an import graph does not contain cyclic imports
 */
let no_import_cycles = (graph: ImportGraph.t) =>
  graph
  |> ImportGraph.find_cycles
  |> (List.map(~@Namespace.pp) |> List.map)
  |> (
    fun
    | [] => Ok()
    | cycles =>
      Error(cycles |> List.map(cycle => AST.Error.ImportCycle(cycle)))
  );

/**
 validate that an import graph does not contain unresolved modules
 */
let no_unresolved_modules = (graph: ImportGraph.t) =>
  graph
  |> ImportGraph.find_missing
  |> List.map(~@Namespace.pp)
  |> (
    fun
    | [] => Ok()
    | missing =>
      Error(missing |> List.map(path => AST.Error.UnresolvedModule(path)))
  );
