open Kore;
open Util;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  open AST.Raw;

  include Assert;
  include Assert.Make({
    type t = module_statement_t;

    let parser = ctx => Parser.parse(Declaration.parser(ctx));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Debug.print_mod_stmt
              % Cow.Xml.list
              % Cow.Xml.to_string
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
              "foo" |> RawUtil.public |> as_lexeme |> RawUtil.named_export,
              nil_prim |> RawUtil.const,
            )
            |> RawUtil.decl,
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
              "foo" |> RawUtil.public |> as_lexeme |> RawUtil.main_export,
              nil_prim |> RawUtil.const,
            )
            |> RawUtil.decl,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
