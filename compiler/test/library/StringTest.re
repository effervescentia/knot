open Kore;

let suite =
  "Library.String"
  >::: [
    "of_uchars()"
    >: (
      () =>
        [
          (
            "abc",
            String.of_uchars([
              Uchar.of_char('a'),
              Uchar.of_char('b'),
              Uchar.of_char('c'),
            ]),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "drop_prefix()"
    >: (
      () =>
        [
          ("bar", String.drop_prefix("foo", "foobar")),
          ("foobar", String.drop_prefix("fizz", "foobar")),
        ]
        |> Assert.(test_many(string))
    ),
    "drop_suffix()"
    >: (
      () =>
        [
          ("foo", String.drop_suffix("bar", "foobar")),
          ("foobar", String.drop_suffix("fizz", "foobar")),
        ]
        |> Assert.(test_many(string))
    ),
    "find_index()"
    >: (
      () =>
        [
          (Some(3), String.find_index("bar", "foobar")),
          (None, String.find_index("fizz", "foobar")),
          (Some(0), String.find_index("", "foobar")),
          (None, String.find_index("foo", "")),
          (Some(0), String.find_index("", "")),
        ]
        |> Assert.(test_many(opt_int))
    ),
    "split()"
    >: (
      () =>
        [(("foo", "bar"), String.split(": ", "foo: bar"))]
        |> Assert.(test_many(string_pair))
    ),
  ];
