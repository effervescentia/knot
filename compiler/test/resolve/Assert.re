open Kore;
open Resolve;

include TestLibrary.Assert;

let dump_program = ppf =>
  Language.Program.program_to_xml(~@AST.Type.pp)
  % Pretty.XML.xml(Fmt.string, ppf);

let import_graph =
  Alcotest.(
    check(
      testable(ImportGraph.pp, ImportGraph.compare),
      "import graph matches",
    )
  );

let source = Alcotest.(check(testable(Source.pp, (==)), "source matches"));

let program =
  Alcotest.(check(testable(dump_program, (==)), "program matches"));
let result_program =
  Alcotest.(
    check(
      result(
        testable(dump_program, (==)),
        testable(AST.Error.pp_compile_err, (==)),
      ),
      "program result matches",
    )
  );

let resolver =
  Alcotest.(check(testable(Resolver.pp, (==)), "resolver matches"));
