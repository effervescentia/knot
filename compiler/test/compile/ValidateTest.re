open Kore;

module Validate = Compile.Validate;

let __id = Internal("foo");

let suite =
  "Compile.Validate"
  >::: [
    "no_import_cycles() - valid"
    >: (
      () => {
        let graph =
          Resolve.ImportGraph.create(__id, id =>
            id == __id ? [Internal("bar")] : []
          );

        Validate.no_import_cycles(graph);
      }
    ),
    "no_import_cycles() - invalid"
    >: (
      () => {
        let graph = Resolve.ImportGraph.create(__id, _ => [__id]);

        Alcotest.check_raises(
          "should throw CompilerError",
          CompilerError(ErrorList([ImportCycle(["@/foo"])])),
          () =>
          Validate.no_import_cycles(graph)
        );
      }
    ),
    "no_unresolved_modules() - valid"
    >: (
      () => {
        let graph =
          Resolve.ImportGraph.create(__id, id =>
            id == __id ? [Internal("bar")] : []
          );

        Validate.no_unresolved_modules(graph);
      }
    ),
    "no_unresolved_modules() - invalid"
    >: (
      () => {
        let other_id = Internal("bar");
        let graph =
          Resolve.ImportGraph.create(__id, id =>
            id == __id ? [other_id] : []
          );

        graph.imports |> Graph.add_edge(__id, other_id);
        graph.imports |> Graph.remove_node(other_id);

        Alcotest.check_raises(
          "should throw CompilerError",
          CompilerError(ErrorList([UnresolvedModule("@/bar")])),
          () =>
          Validate.no_unresolved_modules(graph)
        );
      }
    ),
  ];
