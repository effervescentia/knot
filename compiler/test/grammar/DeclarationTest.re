open Kore;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = A.module_statement_t;

    let parser =
      KDeclaration.Plugin.parse % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Language.Debug.module_statement_to_xml % Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Declaration"
  >::: [
    "parse"
    >: (
      () =>
        Assert.parse(
          (
            AST.ModuleStatement.ExportKind.Named,
            "foo" |> U.as_untyped,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_export
          |> U.as_untyped,
          "const foo = nil",
        )
    ),
    "parse main"
    >: (
      () =>
        Assert.parse(
          (
            AST.ModuleStatement.ExportKind.Main,
            "foo" |> U.as_untyped,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_export
          |> U.as_untyped,
          "main const foo = nil",
        )
    ),
  ];
