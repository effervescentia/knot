open Kore;
open AST;

include Test.Assert;

module Formatter = Grammar.Formatter;

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

let analyzed_expression =
  Alcotest.(
    check(
      testable(Analyzed.Dump.(ppf => expr_to_entity % Entity.pp(ppf)), (==)),
      "analyzed expression matches",
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
