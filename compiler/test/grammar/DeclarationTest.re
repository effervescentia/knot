open Kore;
open Reference;

module Declaration = Grammar.Declaration;
module U = Util.ResultUtilV2;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = A.module_statement_t;

    let parser = ((_, ctx)) =>
      Declaration.parser(ctx) |> Assert.parse_completely |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            ppf => A.Dump.(mod_stmt_to_entity % Entity.pp(ppf)),
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
            "foo" |> A.of_public |> U.as_raw_node |> A.of_named_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_decl
          |> U.as_raw_node,
          "const foo = nil",
        )
    ),
    "parse main"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> A.of_public |> U.as_raw_node |> A.of_main_export,
            U.nil_prim |> A.of_const |> U.as_nil,
          )
          |> A.of_decl
          |> U.as_raw_node,
          "main const foo = nil",
        )
    ),
  ];
