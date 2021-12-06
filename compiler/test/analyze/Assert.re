open Kore;

include Test.Assert;

module Formatter = Grammar.Formatter;

let primitive =
  Alcotest.(
    check(
      testable(pp => Node.get_value % Formatter.pp_prim(pp), (==)),
      "primitive matches",
    )
  );

let expression =
  Alcotest.(
    check(
      testable(pp => Node.get_value % Formatter.pp_expression(pp), (==)),
      "expression matches",
    )
  );

let jsx =
  Alcotest.(
    check(
      testable(pp => Node.get_value % Formatter.pp_jsx(pp), (==)),
      "jsx matches",
    )
  );

let statement =
  Alcotest.(
    check(
      testable(pp => Node.get_value % Formatter.pp_statement(pp), (==)),
      "statement matches",
    )
  );
