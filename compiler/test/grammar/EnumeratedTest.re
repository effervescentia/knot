open Kore;

module U = Util.ResultUtil;

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
          (
            U.as_untyped("foo"),
            [] |> Declaration.of_enumerated |> U.as_enum([]),
          )
          |> U.as_untyped,
          "enum foo =",
        )
    ),
    "parse vertical bar"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            [] |> Declaration.of_enumerated |> U.as_enum([]),
          )
          |> U.as_untyped,
          "enum foo = |",
        )
    ),
    "parse inline with no type parameters"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("foo"),
            [(U.as_untyped("OnlyOption"), [])]
            |> Declaration.of_enumerated
            |> U.as_enum([("OnlyOption", [])]),
          )
          |> U.as_untyped,
          "enum foo = OnlyOption",
        )
    ),
    "parse multiline with type parameters"
    >: (
      () =>
        Assert.Declaration.parse(
          (
            U.as_untyped("Account"),
            [
              (
                U.as_untyped("Verified"),
                [
                  U.as_int(TypeExpression.Integer),
                  U.as_string(TypeExpression.String),
                ],
              ),
              (
                U.as_untyped("Unverified"),
                [TypeExpression.String |> U.as_string],
              ),
            ]
            |> Declaration.of_enumerated
            |> U.as_enum([
                 ("Verified", [Valid(Integer), Valid(String)]),
                 ("Unverified", [Valid(String)]),
               ]),
          )
          |> U.as_untyped,
          "enum Account =
  | Verified(integer, string)
  | Unverified(string)",
        )
    ),
  ];
