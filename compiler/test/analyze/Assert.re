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
        (ppf, (expression, type_)) =>
          Fmt.Node(
            "Expression",
            [("type", type_ |> ~@Type.pp)],
            KExpression.Debug.unpack(~@Type.pp, expression),
          )
          |> Fmt.xml_string(ppf),
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
          KSX.to_xml((KExpression.Plugin.to_xml(~@Type.pp), ~@Type.pp))
          % Fmt.xml_string(ppf),
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
        (ppf, (statement, type_)) =>
          Fmt.Node(
            "Statement",
            [("type", type_ |> ~@Type.pp)],
            KStatement.Debug.unpack(
              (KExpression.Plugin.to_xml(~@Type.pp), ~@Type.pp),
              statement,
            ),
          )
          |> Fmt.xml_string(ppf),
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
          KLambda.Debug.parameter_to_xml(
            KExpression.Plugin.to_xml(~@AST.Type.pp),
            ~@AST.Type.pp,
          )
          % Fmt.xml_string(ppf),
        (==),
      ),
      "argument matches",
      expected,
      actual,
    )
  );
