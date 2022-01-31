open Kore;
open AST;

include Test.Assert;

module Formatter = Grammar.Formatter;
module Analyzer = Analyze.Analyzer;

let weak_type =
  Alcotest.(
    check(
      testable(
        ppf =>
          Fmt.result(
            ~ok=Type.Raw.pp_strong,
            ~error=Type.Error.pp(Type.Raw.pp),
            ppf,
          ),
        (==),
      ),
      "weak type matches",
    )
  );

let analyzed_primitive =
  Alcotest.(
    check(
      testable(Analyzed.Dump.(ppf => prim_to_entity % Entity.pp(ppf)), (==)),
      "analyzed primitive matches",
    )
  );

let primitive =
  Alcotest.(
    check(
      testable(Dump.(ppf => prim_to_entity % Entity.pp(ppf)), (==)),
      "primitive matches",
    )
  );

let analyzed_expression = (expected, actual) =>
  Alcotest.(
    check(
      testable(Analyzed.Dump.(ppf => expr_to_entity % Entity.pp(ppf)), (==)),
      "analyzed expression matches",
      expected,
      actual |> Tuple.join2(Analyzer.analyze_expression),
    )
  );

let expression =
  Alcotest.(
    check(
      testable(Dump.(ppf => expr_to_entity % Entity.pp(ppf)), (==)),
      "expression matches",
    )
  );

let analyzed_jsx =
  Alcotest.(
    check(
      testable(Analyzed.Dump.(ppf => jsx_to_entity % Entity.pp(ppf)), (==)),
      "analyzed jsx matches",
    )
  );

let jsx =
  Alcotest.(
    check(
      testable(Dump.(ppf => jsx_to_entity % Entity.pp(ppf)), (==)),
      "jsx matches",
    )
  );

let analyzed_statement =
  Alcotest.(
    check(
      testable(Analyzed.Dump.(ppf => stmt_to_entity % Entity.pp(ppf)), (==)),
      "analyzed_statement matches",
    )
  );

let statement =
  Alcotest.(
    check(
      testable(Dump.(ppf => stmt_to_entity % Entity.pp(ppf)), (==)),
      "statement matches",
    )
  );

let declaration =
  Alcotest.(
    check(
      testable(Dump.(ppf => decl_to_entity % Entity.pp(ppf)), (==)),
      "declaration matches",
    )
  );
