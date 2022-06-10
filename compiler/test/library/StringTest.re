open Kore;

let suite =
  "Library.String"
  >::: [
    "of_uchars()"
    >: (
      () =>
        Assert.string(
          "abc",
          String.of_uchars([
            Uchar.of_char('a'),
            Uchar.of_char('b'),
            Uchar.of_char('c'),
          ]),
        )
    ),
    "drop_prefix()"
    >: (() => Assert.string("bar", String.drop_prefix("foo", "foobar"))),
    "drop_prefix() - prefix not found"
    >: (() => Assert.string("foobar", String.drop_prefix("fizz", "foobar"))),
    "drop_suffix()"
    >: (() => Assert.string("foo", String.drop_suffix("bar", "foobar"))),
    "drop_suffix() - suffix not found"
    >: (() => Assert.string("foobar", String.drop_suffix("fizz", "foobar"))),
    "drop_all_prefix() - single prefix"
    >: (() => Assert.string("bar", String.drop_all_prefix("foo", "foobar"))),
    "drop_all_prefix() - repeated prefix"
    >: (
      () =>
        Assert.string("bar", String.drop_all_prefix("foo", "foofoofoobar"))
    ),
    "drop_all_prefix() - prefix not found"
    >: (
      () => Assert.string("foobar", String.drop_all_prefix("fizz", "foobar"))
    ),
    "drop_all_suffix() - single suffix"
    >: (() => Assert.string("foo", String.drop_all_suffix("bar", "foobar"))),
    "drop_all_suffix() - repeated suffix"
    >: (
      () =>
        Assert.string("foo", String.drop_all_suffix("bar", "foobarbarbar"))
    ),
    "drop_all_suffix() - suffix not found"
    >: (
      () => Assert.string("foobar", String.drop_all_suffix("fizz", "foobar"))
    ),
    "find_index() - substring"
    >: (() => Assert.opt_int(Some(3), String.find_index("bar", "foobar"))),
    "find_index() - not found"
    >: (() => Assert.opt_int(None, String.find_index("fizz", "foobar"))),
    "find_index() - blank pattern"
    >: (() => Assert.opt_int(Some(0), String.find_index("", "foobar"))),
    "find_index() - blank string"
    >: (() => Assert.opt_int(None, String.find_index("foo", ""))),
    "find_index() - empty string match"
    >: (() => Assert.opt_int(Some(0), String.find_index("", ""))),
    "replace() - pattern found"
    >: (
      () => Assert.string("fiZZbuZZ", String.replace('z', 'Z', "fizzbuzz"))
    ),
    "replace() - pattern not found"
    >: (
      () => Assert.string("fizzbuzz", String.replace('a', 'Z', "fizzbuzz"))
    ),
    "split() - separator found"
    >: (
      () =>
        Assert.string_pair(("foo", "bar"), String.split(": ", "foo: bar"))
    ),
    "split() - separator not found"
    >: (
      () => Assert.string_pair(("foobar", ""), String.split(":", "foobar"))
    ),
    "split() - blank separator"
    >: (
      () => Assert.string_pair(("", "foobar"), String.split("", "foobar"))
    ),
    "split() - empty string"
    >: (() => Assert.string_pair(("", ""), String.split("", ""))),
    "to_kebab_case()"
    >: (
      () =>
        Assert.string(
          "foo-bar-fizz-buzz",
          String.to_kebab_case("foo_bar_fizz_buzz"),
        )
    ),
    "to_pascal_case()"
    >: (
      () =>
        Assert.string(
          "FooBarFizzBuzz",
          String.to_pascal_case("foo_bar_fizz_buzz"),
        )
    ),
  ];
