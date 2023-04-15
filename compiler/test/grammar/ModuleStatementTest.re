open Kore;

module U = Util.ResultUtil;

module Assert =
  Assert.Make({
    type t = ModuleStatement.node_t(Declaration.node_t(Type.t));

    let parser =
      ModuleStatement.parse(Declaration.parse)
      % Assert.parse_completely
      % Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              ModuleStatement.to_xml(Declaration.to_xml(~@Type.pp))
              % Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.ModuleStatement"
  >::: [
    "parse"
    >: (
      () =>
        Assert.parse(
          (
            ExportKind.Named,
            U.as_untyped("foo"),
            U.nil_prim |> Declaration.of_constant |> U.as_nil,
          )
          |> ModuleStatement.of_export
          |> U.as_untyped,
          "const foo = nil",
        )
    ),
    "parse main"
    >: (
      () =>
        Assert.parse(
          (
            ExportKind.Main,
            U.as_untyped("foo"),
            U.nil_prim |> Declaration.of_constant |> U.as_nil,
          )
          |> ModuleStatement.of_export
          |> U.as_untyped,
          "main const foo = nil",
        )
    ),
  ];
