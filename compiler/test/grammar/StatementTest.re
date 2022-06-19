open Kore;

module Expression = Grammar.Expression;
module Statement = Grammar.Statement;
module U = Util.RawUtil;

module Assert =
  Assert.Make({
    type t = AR.statement_t;

    let parser = ((_, ctx)) =>
      Statement.parser(ctx, Expression.parser)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(ppf => AR.Dump.(stmt_to_entity % Entity.pp(ppf)), (==)),
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
          ("foo" |> AR.of_id |> U.as_unknown, U.int_prim(3))
          |> AR.of_add_op
          |> U.as_unknown
          |> AR.of_expr
          |> U.as_unknown,
          ["foo + 3", "foo + 3;"],
        )
    ),
    "parse variable declaration"
    >: (
      () =>
        Assert.parse_all(
          (
            U.as_untyped("foo"),
            ("bar" |> AR.of_id |> U.as_unknown, U.int_prim(3))
            |> AR.of_add_op
            |> U.as_unknown,
          )
          |> AR.of_var
          |> U.as_nil,
          ["let foo = bar + 3", "let foo = bar + 3;"],
        )
    ),
  ];
