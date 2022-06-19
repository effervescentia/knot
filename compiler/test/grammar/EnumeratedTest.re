open Kore;

module Export = Reference.Export;
module Enumerated = Grammar.Enumerated;
module TE = AST.TypeExpression;
module U = Util.ResultUtil;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((A.export_t, A.declaration_t), unit);

    let parser = ((_, ctx)) =>
      Enumerated.parser(ctx, A.of_named_export)
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) => {
              let (export, decl) = fst(stmt);

              A.Dump.(
                untyped_node_to_entity(
                  "Enumerated",
                  ~children=[
                    export |> export_to_entity,
                    decl |> decl_to_entity,
                  ],
                  stmt,
                )
                |> Entity.pp(ppf)
              );
            },
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Declaration | Enumerated"
  >::: [
    "no parse" >: (() => Assert.parse_none(["gibberish", "enum", "enum foo"])),
    "parse empty"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped |> A.of_named_export,
            [] |> A.of_enum |> U.as_enum([]),
          )
          |> U.as_untyped,
          "enum foo =",
        )
    ),
    "parse vertical bar"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped |> A.of_named_export,
            [] |> A.of_enum |> U.as_enum([]),
          )
          |> U.as_untyped,
          "enum foo = |",
        )
    ),
    "parse inline with no type parameters"
    >: (
      () =>
        Assert.parse(
          (
            "foo" |> U.as_untyped |> A.of_named_export,
            [(U.as_untyped("OnlyOption"), [])]
            |> A.of_enum
            |> U.as_enum([("OnlyOption", [])]),
          )
          |> U.as_untyped,
          "enum foo = OnlyOption",
        )
    ),
    "parse multiline with type parameters"
    >: (
      () =>
        Assert.parse(
          (
            "Account" |> U.as_untyped |> A.of_named_export,
            [
              (
                U.as_untyped("Verified"),
                [U.as_int(TE.Integer), U.as_string(TE.String)],
              ),
              (U.as_untyped("Unverified"), [TE.String |> U.as_string]),
            ]
            |> A.of_enum
            |> U.as_enum([
                 ("Verified", [T.Valid(`Integer), T.Valid(`String)]),
                 ("Unverified", [T.Valid(`String)]),
               ]),
          )
          |> U.as_untyped,
          "enum Account =
  | Verified(integer, string)
  | Unverified(string)",
        )
    ),
  ];
