open Kore;

module ImportGraph = Resolve.ImportGraph;
module Validate = Compile.Validate;
module Nx = Fixtures.Namespace;

let __graph =
  ImportGraph.{imports: Fixtures.Graph.two_node, get_imports: _ => []};

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
    >: (() => _assert_valid(Validate.no_import_cycles(__graph))),
    "no_import_cycles() - invalid"
    >: (
      () => {
        let graph = {
          ...__graph,
          imports: Graph.create([Nx.foo], [(Nx.foo, Nx.foo)]),
        };

        _assert_invalid(
          [ImportCycle(["@/foo"])],
          Validate.no_import_cycles(graph),
        );
      }
    ),
    "no_unresolved_modules() - valid"
    >: (() => _assert_valid(Validate.no_unresolved_modules(__graph))),
    "no_unresolved_modules() - invalid"
    >: (
      () => {
        let graph = {
          ...__graph,
          imports: Graph.create([Nx.foo], [(Nx.foo, Nx.bar)]),
        };

        _assert_invalid(
          [UnresolvedModule("@/bar")],
          Validate.no_unresolved_modules(graph),
        );
      }
    ),
  ];
