open Kore;
open AST;

include Test.Assert;

module Formatter = Grammar.Formatter;

let primitive =
  Alcotest.(
    check(
      testable(ppf => Dump.prim_to_entity % Dump.Entity.pp(ppf), (==)),
      "primitive matches",
    )
  );

let expression =
  Alcotest.(
    check(
      testable(ppf => Dump.expr_to_entity % Dump.Entity.pp(ppf), (==)),
      "expression matches",
    )
  );

let jsx =
  Alcotest.(
    check(
      testable(ppf => Dump.jsx_to_entity % Dump.Entity.pp(ppf), (==)),
      "jsx matches",
    )
  );

let statement =
  Alcotest.(
    check(
      testable(ppf => Dump.stmt_to_entity % Dump.Entity.pp(ppf), (==)),
      "statement matches",
    )
  );

let declaration =
  Alcotest.(
    check(
      testable(ppf => Dump.decl_to_entity % Dump.Entity.pp(ppf), (==)),
      "declaration matches",
    )
  );
