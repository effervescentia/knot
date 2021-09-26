open Kore;

module Validate = Compile.Validate;

let __id = Reference.Namespace.Internal("foo");

let _setup = get_imports => {
  let graph = Resolve.ImportGraph.create(get_imports);

  graph |> Resolve.ImportGraph.init(__id);

  graph;
};

let _assert_validate_result =
  Alcotest.(
    check(
      result(
        testable((_, _) => (), (==)),
        testable(pp_dump_err_list, (==)),
      ),
      "validate result matches",
    )
  );
let _assert_valid = _assert_validate_result(Ok());
let _assert_invalid = errs => _assert_validate_result(Error(errs));

let suite =
  "Compile.Validate"
  >::: [
    "no_import_cycles() - valid"
    >: (
      () => {
        let graph = _setup(id => id == __id ? [Internal("bar")] : []);

        _assert_valid(Validate.no_import_cycles(graph));
      }
    ),
    "no_import_cycles() - invalid"
    >: (
      () => {
        let graph = _setup(_ => [__id]);

        _assert_invalid(
          [ImportCycle(["@/foo"])],
          Validate.no_import_cycles(graph),
        );
      }
    ),
    "no_unresolved_modules() - valid"
    >: (
      () => {
        let graph = _setup(id => id == __id ? [Internal("bar")] : []);

        _assert_valid(Validate.no_unresolved_modules(graph));
      }
    ),
    "no_unresolved_modules() - invalid"
    >: (
      () => {
        let other_id = Reference.Namespace.Internal("bar");
        let graph = _setup(id => id == __id ? [other_id] : []);

        graph.imports |> Graph.add_edge(__id, other_id);
        graph.imports |> Graph.remove_node(other_id);

        _assert_invalid(
          [UnresolvedModule("@/bar")],
          Validate.no_unresolved_modules(graph),
        );
      }
    ),
  ];
