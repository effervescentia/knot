open Kore;

include TestLibrary.Assert;

module Formatter = Grammar.Formatter;

let type_error =
  Alcotest.(
    check(
      testable(ppf => Fmt.option(~none=Fmt.nop, Type.pp_error, ppf), (==)),
      "type error matches",
    )
  );

let expression = (expected, actual) =>
  Alcotest.(
    check(
      testable(AST.Dump.(ppf => expr_to_entity % Entity.pp(ppf)), (==)),
      "expression matches",
      expected,
      actual,
    )
  );

let jsx = (expected, actual) =>
  Alcotest.(
    check(
      testable(AST.Dump.(ppf => jsx_to_entity % Entity.pp(ppf)), (==)),
      "jsx matches",
      expected,
      actual,
    )
  );

let statement = (expected, actual) =>
  Alcotest.(
    check(
      testable(AST.Dump.(ppf => stmt_to_entity % Entity.pp(ppf)), (==)),
      "statement matches",
      expected,
      actual,
    )
  );
