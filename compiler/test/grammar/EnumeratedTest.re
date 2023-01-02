open Kore;

module Export = Reference.Export;

let suite =
  "Grammar.Enumerated"
  >::: [
    "no parse"
    >: (
      () => Assert.Declaration.parse_none(["gibberish", "enum", "enum foo"])
    ),
    "parse empty"
    >: (
      () =>
        Assert.Declaration.parse(
          (AM.Named, U.as_untyped("foo"), [] |> A.of_enum |> U.as_enum([]))
          |> A.of_export
          |> U.as_untyped,
          "enum foo =",
        )
    ),
    "parse vertical bar"
    >: (
      () =>
        Assert.Declaration.parse(
          (AM.Named, U.as_untyped("foo"), [] |> A.of_enum |> U.as_enum([]))
          |> A.of_export
          |> U.as_untyped,
          "enum foo = |",
        )
    ),
    "parse inline with no type parameters"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            AM.Named,
            U.as_untyped("foo"),
            [(U.as_untyped("OnlyOption"), [])]
            |> A.of_enum
            |> U.as_enum([("OnlyOption", [])]),
          )
          |> A.of_export
          |> U.as_untyped,
          "enum foo = OnlyOption",
        )
    ),
    "parse multiline with type parameters"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            AM.Named,
            U.as_untyped("Account"),
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
          |> A.of_export
          |> U.as_untyped,
          "enum Account =
  | Verified(integer, string)
  | Unverified(string)",
        )
    ),
  ];
