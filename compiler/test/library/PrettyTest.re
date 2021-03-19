open Kore;

let suite =
  "Library.Pretty"
  >::: [
    "to_string()"
    >: (
      () =>
        Pretty.[
          ("", Nil |> to_string),
          ("", (Nil, Nil) |> append |> to_string),
          (
            "foobar",
            ("foo" |> string, "bar" |> string) |> append |> to_string,
          ),
          (
            "foobar\nfizzbuzz",
            ["foobar" |> string, Newline, "fizzbuzz" |> string]
            |> concat
            |> to_string,
          ),
          (
            "foo {\n  bar {\n    fizz;\n    buzz;\n  }\n}",
            [
              ["foo" |> string, " {" |> string] |> newline,
              [
                ["bar" |> string, " {" |> string] |> newline,
                [
                  ["fizz" |> string, ";" |> string] |> newline,
                  ["buzz" |> string, ";" |> string] |> newline,
                ]
                |> concat
                |> indent(2),
                ["}" |> string] |> newline,
              ]
              |> concat
              |> indent(2),
              "}" |> string,
            ]
            |> concat
            |> to_string,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
