open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = module_statement_t;

    let parser = ((_, ctx)) => Parser.parse(Declaration.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            ppf => Dump.mod_stmt_to_entity % Dump.Entity.pp(ppf),
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
            "foo" |> of_public |> as_raw_node |> of_named_export,
            nil_prim |> of_const |> as_nil,
          )
          |> of_decl
          |> as_raw_node,
          "const foo = nil",
        )
    ),
    "parse main"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> of_public |> as_raw_node |> of_main_export,
            nil_prim |> of_const |> as_nil,
          )
          |> of_decl
          |> as_raw_node,
          "main const foo = nil",
        )
    ),
  ];
