open Kore;
open AST.Raw.Util;
open Reference;
open Util;

module Declaration = Grammar.Declaration;

module Assert = {
  open AST.Raw;

  include Assert;
  include Assert.Make({
    type t = (export_t, declaration_t);

    let parser = ctx =>
      Parser.parse(Declaration.function_(ctx, to_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Debug.print_decl
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
  "Grammar.Declaration (Function)"
  >::: [
    "no parse"
    >: (
      () =>
        [
          "gibberish",
          "func",
          "func foo",
          "func foo ()",
          "func foo () ->",
          "func foo () -> {",
          "func foo ->",
          "func foo -> {",
        ]
        |> Assert.no_parse
    ),
    "parse"
    >: (
      () =>
        [
          (
            "func foo -> nil",
            (to_public_export("foo"), ([], nil_prim) |> to_func),
          ),
          (
            "func foo -> { nil }",
            (
              to_public_export("foo"),
              ([], [to_expr(nil_prim)] |> to_closure |> as_nil) |> to_func,
            ),
          ),
          (
            "func foo () -> nil",
            (to_public_export("foo"), ([], nil_prim) |> to_func),
          ),
          (
            "func foo () -> { nil }",
            (
              to_public_export("foo"),
              ([], [to_expr(nil_prim)] |> to_closure |> as_nil) |> to_func,
            ),
          ),
        ]
        |> Assert.parse_many
    ),
  ];
