open Kore;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = AM.module_statement_t;

    let parser =
      KDeclaration.Plugin.parse % Assert.parse_completely % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Language.Program.module_statement_to_xml(~@AST.Type.pp)
              % Fmt.xml_string(ppf),
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
            "foo" |> U.as_untyped |> A.of_named_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_decl
          |> U.as_untyped,
          "const foo = nil",
        )
    ),
    "parse main"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped |> A.of_main_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_decl
          |> U.as_untyped,
          "main const foo = nil",
        )
    ),
  ];
