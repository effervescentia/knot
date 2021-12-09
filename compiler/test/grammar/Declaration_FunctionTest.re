open Kore;
open AST;
open Util.ResultUtil;
open Reference;

module Declaration = Grammar.Declaration;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = Node.Raw.t((export_t, declaration_t));

    let parser = ((_, ctx)) =>
      Parser.parse(Declaration.function_(ctx, AST.of_named_export));

    let test =
      Alcotest.(
        check(
          testable(
            (pp, stmt) => {
              let (export, decl) = Node.Raw.get_value(stmt);

              Dump.raw_node_to_entity(
                "Declaration",
                ~children=[
                  export |> Dump.export_to_entity,
                  decl |> Dump.decl_to_entity,
                ],
                stmt,
              )
              |> Dump.Entity.pp(pp);
            },
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
              "foo" |> of_public |> as_raw_node |> of_named_export,
              ([], nil_prim) |> of_func |> as_function([], Valid(`Nil)),
            )
            |> as_raw_node,
          ),
          (
            "func foo -> { nil }",
            (
              "foo" |> of_public |> as_raw_node |> of_named_export,
              ([], [nil_prim |> of_expr |> as_nil] |> of_closure |> as_nil)
              |> of_func
              |> as_function([], Valid(`Nil)),
            )
            |> as_raw_node,
          ),
          (
            "func foo () -> nil",
            (
              "foo" |> of_public |> as_raw_node |> of_named_export,
              ([], nil_prim) |> of_func |> as_function([], Valid(`Nil)),
            )
            |> as_raw_node,
          ),
          (
            "func foo () -> { nil }",
            (
              "foo" |> of_public |> as_raw_node |> of_named_export,
              ([], [nil_prim |> of_expr |> as_nil] |> of_closure |> as_nil)
              |> of_func
              |> as_function([], Valid(`Nil)),
            )
            |> as_raw_node,
          ),
        ]
        |> Assert.parse_many
    ),
  ];
