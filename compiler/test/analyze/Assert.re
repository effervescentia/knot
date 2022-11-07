open Kore;

include TestLibrary.Assert;

module Formatter = Language.Formatter;
module Type = AST.Type;

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
      testable(
        ppf =>
          KExpression.Plugin.to_xml(~@Type.pp) % Fmt.xml(Fmt.string, ppf),
        (==),
      ),
      "expression matches",
      expected,
      actual,
    )
  );

let jsx = (expected, actual) =>
  Alcotest.(
    check(
      testable(
        ppf =>
          KSX.Plugin.to_xml(KExpression.Plugin.to_xml(~@Type.pp), ~@Type.pp)
          % Fmt.xml(Fmt.string, ppf),
        (==),
      ),
      "jsx matches",
      expected,
      actual,
    )
  );

let statement = (expected, actual) =>
  Alcotest.(
    check(
      testable(
        ppf =>
          KStatement.Plugin.to_xml(
            KExpression.Plugin.to_xml(~@Type.pp),
            ~@Type.pp,
          )
          % Fmt.xml(Fmt.string, ppf),
        (==),
      ),
      "statement matches",
      expected,
      actual,
    )
  );

let argument = (expected, actual) =>
  Alcotest.(
    check(
      testable(
        ppf =>
          KLambda.Plugin.argument_to_xml(
            KExpression.Plugin.to_xml(~@AST.Type.pp),
            ~@AST.Type.pp,
          )
          % Fmt.xml(Fmt.string, ppf),
        (==),
      ),
      "argument matches",
      expected,
      actual,
    )
  );
