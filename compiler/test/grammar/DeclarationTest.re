open Kore;
open Reference;

module Declaration = Grammar.Declaration;
module U = Util.ResultUtil;

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
