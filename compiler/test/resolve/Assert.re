open Kore;
open Resolve;

include TestLibrary.Assert;

let import_graph =
  Alcotest.(
    check(
      testable(ImportGraph.pp, ImportGraph.compare),
      "import graph matches",
    )
  );

let source = Alcotest.(check(testable(Source.pp, (==)), "source matches"));

let program =
  Alcotest.(check(testable(AST.Result.Dump.pp, (==)), "program matches"));
let result_program =
  Alcotest.(
    check(
      result(
        testable(AST.Result.Dump.pp, (==)),
        testable(AST.Error.pp_compile_err, (==)),
      ),
      "program result matches",
    )
  );

let resolver =
  Alcotest.(check(testable(Resolver.pp, (==)), "resolver matches"));
