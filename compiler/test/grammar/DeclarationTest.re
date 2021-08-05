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
            pp =>
              Debug.print_mod_stmt
              % Pretty.to_string
              % Format.pp_print_string(pp),
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
        [
          (
            "const foo = nil",
            (
              "foo" |> of_public |> as_raw_node |> of_named_export,
              nil_prim |> of_const |> as_nil,
            )
            |> of_decl
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
    "parse main"
    >: (
      () =>
        [
          (
            "main const foo = nil",
            (
              "foo" |> of_public |> as_raw_node |> of_main_export,
              nil_prim |> of_const |> as_nil,
            )
            |> of_decl
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
