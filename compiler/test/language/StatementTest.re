open Kore;

module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = Statement.node_t(Expression.t(unit), unit);

    let parser = ctx =>
      Statement.parse(ctx, Expression.parse)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf =>
              Statement.to_xml((
                Expression.to_xml(_ => "Unknown"),
                _ => "Unknown",
              ))
              % Fmt.xml_string(ppf),
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
          ("foo" |> Expression.of_identifier |> U.as_node, U.int_prim(3))
          |> Expression.of_add_op
          |> U.as_node
          |> Statement.of_effect
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
            ("bar" |> Expression.of_identifier |> U.as_node, U.int_prim(3))
            |> Expression.of_add_op
            |> U.as_node,
          )
          |> Statement.of_variable
          |> U.as_node,
          ["let foo = bar + 3", "let foo = bar + 3;"],
        )
    ),
  ];
