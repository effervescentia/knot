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
    "drop_all_prefix()"
    >: (
      () =>
        [
          ("bar", String.drop_all_prefix("foo", "foobar")),
          ("bar", String.drop_all_prefix("foo", "foofoofoobar")),
          ("foobar", String.drop_all_prefix("fizz", "foobar")),
        ]
        |> Assert.(test_many(string))
    ),
    "drop_all_suffix()"
    >: (
      () =>
        [
          ("foo", String.drop_all_suffix("bar", "foobar")),
          ("foo", String.drop_all_suffix("bar", "foobarbarbar")),
          ("foobar", String.drop_all_suffix("fizz", "foobar")),
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
    "replace()"
    >: (
      () =>
        [
          ("fiZZbuZZ", String.replace('z', 'Z', "fizzbuzz")),
          ("fizzbuzz", String.replace('a', 'Z', "fizzbuzz")),
        ]
        |> Assert.(test_many(string))
    ),
    "split()"
    >: (
      () =>
        [
          (("foo", "bar"), String.split(": ", "foo: bar")),
          (("foobar", ""), String.split(":", "foobar")),
          (("", "foobar"), String.split("", "foobar")),
          (("", ""), String.split("", "")),
        ]
        |> Assert.(test_many(string_pair))
    ),
  ];
