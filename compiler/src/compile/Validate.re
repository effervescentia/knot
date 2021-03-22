/**
 Static analysis validators for compiler state.
 */
open Kore;

/**
 validate that an import graph does not contain cyclic imports
 */
let no_import_cycles = (~catch=throw_all, graph: ImportGraph.t) => {
  graph
  |> ImportGraph.find_cycles
  |> (List.map(AST.string_of_namespace) |> List.map)
  |> (
    fun
    | [] => ()
    | cycles => catch(cycles |> List.map(cycle => ImportCycle(cycle)))
  );
};

/**
 validate that an import graph does not contain unresolved modules
 */
let no_unresolved_modules = (~catch=throw_all, graph: ImportGraph.t) => {
  graph
  |> ImportGraph.find_missing
  |> List.map(AST.string_of_namespace)
  |> (
    fun
    | [] => ()
    | missing =>
      throw_all(missing |> List.map(path => UnresolvedModule(path)))
  );
};
