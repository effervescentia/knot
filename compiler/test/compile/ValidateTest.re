open Kore;

module Validate = Compile.Validate;

let __id = Internal("foo");

let _setup = get_imports => {
  let graph = Resolve.ImportGraph.create(get_imports);

  graph |> Resolve.ImportGraph.init(__id);

  graph;
};

let suite =
  "Compile.Validate"
  >::: [
    "no_import_cycles() - valid"
    >: (
      () => {
        let graph = _setup(id => id == __id ? [Internal("bar")] : []);

        Validate.no_import_cycles(graph);
      }
    ),
    "no_import_cycles() - invalid"
    >: (
      () => {
        let graph = _setup(_ => [__id]);

        Alcotest.check_raises(
          "should throw CompilerError",
          CompilerError([ImportCycle(["@/foo"])]),
          () =>
          Validate.no_import_cycles(graph)
        );
      }
    ),
    "no_unresolved_modules() - valid"
    >: (
      () => {
        let graph = _setup(id => id == __id ? [Internal("bar")] : []);

        Validate.no_unresolved_modules(graph);
      }
    ),
    "no_unresolved_modules() - invalid"
    >: (
      () => {
        let other_id = Internal("bar");
        let graph = _setup(id => id == __id ? [other_id] : []);

        graph.imports |> Graph.add_edge(__id, other_id);
        graph.imports |> Graph.remove_node(other_id);

        Alcotest.check_raises(
          "should throw CompilerError",
          CompilerError([UnresolvedModule("@/bar")]),
          () =>
          Validate.no_unresolved_modules(graph)
        );
      }
    ),
  ];
