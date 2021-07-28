open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = Node.Raw.t((export_t, declaration_t));

    let parser = ((_, ctx, _)) =>
      Parser.parse(Declaration.function_(ctx, AST.of_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Node.Raw.value
              % Debug.print_decl
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
            (
              "foo" |> of_public |> as_lexeme |> of_named_export,
              ([], nil_prim) |> of_func |> as_function([], Valid(`Nil)),
            )
            |> as_lexeme,
          ),
          (
            "func foo -> { nil }",
            (
              "foo" |> of_public |> as_lexeme |> of_named_export,
              ([], [nil_prim |> of_expr] |> of_closure |> as_nil)
              |> of_func
              |> as_function([], Valid(`Nil)),
            )
            |> as_lexeme,
          ),
          (
            "func foo () -> nil",
            (
              "foo" |> of_public |> as_lexeme |> of_named_export,
              ([], nil_prim) |> of_func |> as_function([], Valid(`Nil)),
            )
            |> as_lexeme,
          ),
          (
            "func foo () -> { nil }",
            (
              "foo" |> of_public |> as_lexeme |> of_named_export,
              ([], [nil_prim |> of_expr] |> of_closure |> as_nil)
              |> of_func
              |> as_function([], Valid(`Nil)),
            )
            |> as_lexeme,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
