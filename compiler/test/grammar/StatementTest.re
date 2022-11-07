open Kore;

module AR = AST.Raw;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.statement_t;

    let parser = ctx =>
      KStatement.Plugin.parse(ctx, KExpression.Plugin.parse)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              KStatement.Plugin.to_xml(
                KExpression.Plugin.to_xml(_ => "Unknown"), _ =>
                "Unknown"
              )
              % Fmt.xml(Fmt.string, ppf),
            (==),
          ),
          "program matches",
        )
      );
  });

let suite =
  "Grammar.Statement"
  >::: [
    "no parse" >: (() => Assert.no_parse("~gibberish")),
    "parse expression"
    >: (
      () =>
        Assert.parse_all(
          ("foo" |> AR.of_id |> U.as_node, U.int_prim(3))
          |> AR.of_add_op
          |> U.as_node
          |> AR.of_expr
          |> U.as_node,
          ["foo + 3", "foo + 3;"],
        )
    ),
    "parse variable declaration"
    >: (
      () =>
        Assert.parse_all(
          (
            U.as_untyped("foo"),
            ("bar" |> AR.of_id |> U.as_node, U.int_prim(3))
            |> AR.of_add_op
            |> U.as_node,
          )
          |> AR.of_var
          |> U.as_node,
          ["let foo = bar + 3", "let foo = bar + 3;"],
        )
    ),
  ];
