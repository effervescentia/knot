/**
 Static analysis validators for compiler state.
 */
open Knot.Kore;

module Error = AST.Error;
module ImportGraph = Resolve.ImportGraph;
module Namespace = Reference.Namespace;

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
    | cycles => Error(cycles |> List.map(cycle => Error.ImportCycle(cycle)))
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
      Error(missing |> List.map(path => Error.UnresolvedModule(path)))
  );
