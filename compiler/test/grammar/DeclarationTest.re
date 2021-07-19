open Kore;
open AST.Raw.Util;
open Reference;
open Util;

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
            (to_public_export("foo"), to_const(nil_prim)) |> to_decl,
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
            (to_public_main_export("foo"), to_const(nil_prim)) |> to_decl,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
