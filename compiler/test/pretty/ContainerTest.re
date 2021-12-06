open Kore;
open Fmt;
open Pretty.Container;

let __words = [
  "Sint",
  "eiusmod",
  "quis",
  "consectetur",
  "cillum",
  "nulla",
  "est",
  "et",
  "ipsum",
  "nisi",
];
let __long_words = ["Lorem", "ea", ...__words];

let suite =
  "Pretty.Container"
  >::: [
    "root()"
    >: (
      () =>
        [("<foo></foo>", "<foo></foo>" |> ~@root(string))]
        |> Assert.(test_many(string))
    ),
    "page()"
    >: (
      () =>
        [("<foo></foo>
", "<foo></foo>" |> ~@page(string))]
        |> Assert.(test_many(string))
    ),
    "list()"
    >: (
      () =>
        [
          ("", [] |> ~@root(list(string))),
          ("a, b, c", ["a", "b", "c"] |> ~@root(box(list(string)))),
          ("a, b, c,
", ["a", "b", "c"] |> ~@root(list(string))),
          (
            "a; b; c",
            ["a", "b", "c"]
            |> ~@root(box(list(~sep=Sep.of_sep(";"), string))),
          ),
          (
            "Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi",
            __words |> ~@root(box(list(string))),
          ),
          (
            "Lorem,
ea,
Sint,
eiusmod,
quis,
consectetur,
cillum,
nulla,
est,
et,
ipsum,
nisi,
",
            __long_words |> ~@root(list(string)),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "collection()"
    >: (
      () => {
        let set = x => box(collection(any("<"), any(">"), x));
        let string_set = set(string);
        let string_set_set = set(string_set);

        [
          ("<>", [] |> ~@root(string_set)),
          ("<a, b, c>", ["a", "b", "c"] |> ~@root(string_set)),
          (
            "<Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>",
            __words |> ~@root(string_set),
          ),
          (
            "<
  Lorem,
  ea,
  Sint,
  eiusmod,
  quis,
  consectetur,
  cillum,
  nulla,
  est,
  et,
  ipsum,
  nisi,
>",
            __long_words |> ~@root(string_set),
          ),
          (
            "<<a, b, c>, <d, e, f>>",
            [["a", "b", "c"], ["d", "e", "f"]] |> ~@root(string_set_set),
          ),
          (
            "<
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
>",
            [__words, __words] |> ~@root(box(string_set_set)),
          ),
          (
            "<
  <
    Lorem,
    ea,
    Sint,
    eiusmod,
    quis,
    consectetur,
    cillum,
    nulla,
    est,
    et,
    ipsum,
    nisi,
  >,
  <
    Lorem,
    ea,
    Sint,
    eiusmod,
    quis,
    consectetur,
    cillum,
    nulla,
    est,
    et,
    ipsum,
    nisi,
  >,
>",
            [__long_words, __long_words] |> ~@root(box(string_set_set)),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "array()"
    >: (
      () =>
        [
          ("[]", [] |> ~@root(array(string))),
          ("[a, b, c]", ["a", "b", "c"] |> ~@root(array(string))),
        ]
        |> Assert.(test_many(string))
    ),
    "tuple()"
    >: (
      () =>
        [
          ("()", [] |> ~@root(tuple(string))),
          ("(a, b, c)", ["a", "b", "c"] |> ~@root(tuple(string))),
        ]
        |> Assert.(test_many(string))
    ),
    "closure()"
    >: (
      () =>
        [
          ("{ }", [] |> ~@root(closure(string))),
          ("{
  foo
}", ["foo"] |> ~@root(closure(string))),
          (
            "{
  foo
  bar
  fizz
  buzz
}",
            ["foo", "bar", "fizz", "buzz"] |> ~@root(closure(string)),
          ),
          (
            "{
  {
    foo
    bar
  }
  {
    fizz
    buzz
  }
}",
            [["foo", "bar"], ["fizz", "buzz"]]
            |> ~@root(closure(closure(string))),
          ),
          (
            "prefix {
  foo
}",
            ["foo"] |> ~@root(ppf => pf(ppf, "prefix %a", closure(string))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "attribute()"
    >: (
      () =>
        [
          ("foo: bar", ("foo", "bar") |> ~@root(attribute(string, string))),
          (
            "foo: Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
            (
              "foo",
              "Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
            )
            |> ~@root(attribute(string, string)),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "record()"
    >: (
      () =>
        [
          ("{ }", [] |> ~@root(record(string, string))),
          (
            "{
  foo: bar
}",
            [("foo", "bar")] |> ~@root(record(string, string)),
          ),
          (
            "{
  foo: bar
  fizz: buzz
  zipp: zapp
}",
            [("foo", "bar"), ("fizz", "buzz"), ("zipp", "zapp")]
            |> ~@root(record(string, string)),
          ),
          (
            "{
  foo: Est fugiat nostrud aliquip irure amet commodo voluptate excepteur pariatur esse ut mollit nisi exercitation
}",
            [
              (
                "foo",
                "Est fugiat nostrud aliquip irure amet commodo voluptate excepteur pariatur esse ut mollit nisi exercitation",
              ),
            ]
            |> ~@root(record(string, string)),
          ),
          (
            "{
  foo: {
    fizz: buzz
  }
  bar: {
    zipp: zapp
  }
}",
            [("foo", [("fizz", "buzz")]), ("bar", [("zipp", "zapp")])]
            |> ~@root(record(string, record(string, string))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "uchar()"
    >: (
      () =>
        [
          ("\\u{1f}", Uchar.of_int(31) |> ~@uchar),
          (" ", Uchar.of_int(32) |> ~@uchar),
          ("~", Uchar.of_int(126) |> ~@uchar),
          ("\\u{7f}", Uchar.of_int(127) |> ~@uchar),
          ("\\t", Uchar.of_int(9) |> ~@uchar),
          ("\\n", Uchar.of_int(10) |> ~@uchar),
          ("\\r", Uchar.of_int(13) |> ~@uchar),
        ]
        |> Assert.(test_many(string))
    ),
    "entity()"
    >: (
      () =>
        [
          ("foo { }", ("foo", []) |> ~@root(entity(string, string))),
          (
            "foo {
  a
  b
  c
}",
            ("foo", ["a", "b", "c"]) |> ~@root(entity(string, string)),
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
            |> ~@root(entity(string, entity(string, string))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "struct_()"
    >: (
      () =>
        [
          ("foo { }", ("foo", []) |> ~@root(struct_(string, string))),
          (
            "foo {
  a: 1
  b: 2
  c: 3
}",
            ("foo", [("a", 1), ("b", 2), ("c", 3)])
            |> ~@root(struct_(string, int)),
          ),
          (
            "Hashtbl {
  foo: bar
}",
            ("Hashtbl", [("foo", "bar")])
            |> ~@root(struct_(string, string)),
          ),
          (
            "foo {
  bar: Bar {
    a: 1
    b: 2
    c: 3
  }
  fizz: Fizz {
    d: 4
    e: 5
    f: 6
  }
}",
            (
              "foo",
              [
                ("bar", ("Bar", [("a", 1), ("b", 2), ("c", 3)])),
                ("fizz", ("Fizz", [("d", 4), ("e", 5), ("f", 6)])),
              ],
            )
            |> ~@root(struct_(string, struct_(string, int))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
