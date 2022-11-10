open Kore;

module Export = Reference.Export;

module Assert = {
  include Assert;
  include Assert.Make({
    type t = N.t((AM.export_t, AM.declaration_t), unit);

    let parser = ctx =>
      (ctx, A.of_named_export)
      |> KEnumerated.Plugin.parse
      |> Assert.parse_completely
      |> Parser.parse;

    let test =
      Alcotest.(
        check(
          testable(
            (ppf, stmt) =>
              KDeclaration.Plugin.to_xml(~@Type.pp, fst(stmt))
              |> Fmt.xml_string(ppf),
            (==),
          ),
          "program matches",
        )
      );
  });
};

let suite =
  "Grammar.Enumerated"
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
