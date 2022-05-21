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

let module_ = Alcotest.(check(testable(Module.pp, (==)), "module matches"));

let program =
  Alcotest.(check(testable(AST.Dump.pp, (==)), "program matches"));
let result_program =
  Alcotest.(
    check(
      result(testable(AST.Dump.pp, (==)), testable(pp_compile_err, (==))),
      "program result matches",
    )
  );

let resolver =
  Alcotest.(check(testable(Resolver.pp, (==)), "resolver matches"));
