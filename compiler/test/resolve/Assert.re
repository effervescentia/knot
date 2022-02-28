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

let programV2 =
  Alcotest.(check(testable(ASTV2.Dump.pp, (==)), "program matches"));

let resolver =
  Alcotest.(check(testable(Resolver.pp, (==)), "resolver matches"));
