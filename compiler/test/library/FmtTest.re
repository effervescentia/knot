open Kore;

let suite =
  "Library.Fmt"
  >::: [
    "uchar()"
    >: (
      () =>
        [
          ("\\u{1f}", Uchar.of_int(31) |> ~@Fmt.uchar),
          (" ", Uchar.of_int(32) |> ~@Fmt.uchar),
          ("~", Uchar.of_int(126) |> ~@Fmt.uchar),
          ("\\u{7f}", Uchar.of_int(127) |> ~@Fmt.uchar),
          ("\\t", Uchar.of_int(9) |> ~@Fmt.uchar),
          ("\\n", Uchar.of_int(10) |> ~@Fmt.uchar),
          ("\\r", Uchar.of_int(13) |> ~@Fmt.uchar),
        ]
        |> Assert.(test_many(string))
    ),
    "root()"
    >: (
      () =>
        [("<foo></foo>", "<foo></foo>" |> ~@Fmt.root(Fmt.string))]
        |> Assert.(test_many(string))
    ),
    "block()"
    >: (
      () =>
        [
          (
            "{
  foo
}",
            "foo" |> ~@Fmt.(root(ppf => pf(ppf, "{%a}", block(string)))),
          ),
          (
            "{
  [
    foo
  ]
}",
            "foo"
            |> ~@
                 Fmt.(
                   root(ppf =>
                     pf(
                       ppf,
                       "{%a}",
                       block(ppf => pf(ppf, "[%a]", block(string))),
                     )
                   )
                 ),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "arguments()"
    >: (
      () =>
        [
          ("()", [] |> ~@Fmt.(root(arguments(string)))),
          (
            "(a, b, c)",
            ["a", "b", "c"] |> ~@Fmt.(root(arguments(string))),
          ),
          (
            "((a, b, c), (1, 2, 3))",
            [["a", "b", "c"], ["1", "2", "3"]]
            |> ~@Fmt.(root(arguments(arguments(string)))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "closure()"
    >: (
      () =>
        [
          ("{ }", [] |> ~@Fmt.(root(closure(string)))),
          (
            "{
  a
  b
  c
}",
            ["a", "b", "c"] |> ~@Fmt.(root(closure(string))),
          ),
          (
            "{
  {
    a
    b
    c
  }
  {
    1
    2
    3
  }
}",
            [["a", "b", "c"], ["1", "2", "3"]]
            |> ~@Fmt.(root(closure(closure(string)))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "entity()"
    >: (
      () =>
        [
          ("foo { }", ("foo", []) |> ~@Fmt.(root(entity(string, string)))),
          (
            "foo {
  a
  b
  c
}",
            ("foo", ["a", "b", "c"])
            |> ~@Fmt.(root(entity(string, string))),
          ),
          (
            "foo {
  bar {
    a
    b
    c
  }
  fizz {
    1
    2
    3
  }
}",
            (
              "foo",
              [("bar", ["a", "b", "c"]), ("fizz", ["1", "2", "3"])],
            )
            |> ~@Fmt.(root(entity(string, entity(string, string)))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
