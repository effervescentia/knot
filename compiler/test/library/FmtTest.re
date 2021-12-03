open Kore;

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
  "Library.Fmt"
  >::: [
    "root()"
    >: (
      () =>
        [("<foo></foo>", "<foo></foo>" |> Fmt.(str("%a", root(string))))]
        |> Assert.(test_many(string))
    ),
    "page()"
    >: (
      () =>
        [("<foo></foo>
", "<foo></foo>" |> Fmt.(str("%a", page(string))))]
        |> Assert.(test_many(string))
    ),
    "list()"
    >: (
      () =>
        [
          ("", [] |> (Fmt.(root(list(string))) |> Fmt.to_to_string)),
          (
            "a, b, c",
            ["a", "b", "c"]
            |> (Fmt.(root(box(list(string)))) |> Fmt.to_to_string),
          ),
          (
            "a, b, c,
",
            ["a", "b", "c"]
            |> (Fmt.(root(list(string))) |> Fmt.to_to_string),
          ),
          (
            "a; b; c",
            ["a", "b", "c"]
            |> (
              Fmt.(root(box(list(~sep=Sep.of_sep(";"), string))))
              |> Fmt.to_to_string
            ),
          ),
          (
            "Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi",
            __words |> (Fmt.(root(box(list(string)))) |> Fmt.to_to_string),
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
            __long_words |> (Fmt.(root(list(string))) |> Fmt.to_to_string),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "collection()"
    >: (
      () => {
        let set = x => Fmt.(box(collection(any("<"), any(">"), x)));
        let string_set = set(Fmt.string);
        let string_set_set = set(string_set);

        [
          ("<>", [] |> (Fmt.(root(string_set)) |> Fmt.to_to_string)),
          (
            "<a, b, c>",
            ["a", "b", "c"] |> (Fmt.(root(string_set)) |> Fmt.to_to_string),
          ),
          (
            "<Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>",
            __words |> (Fmt.(root(string_set)) |> Fmt.to_to_string),
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
            __long_words |> (Fmt.(root(string_set)) |> Fmt.to_to_string),
          ),
          (
            "<<a, b, c>, <d, e, f>>",
            [["a", "b", "c"], ["d", "e", "f"]]
            |> (Fmt.(root(string_set_set)) |> Fmt.to_to_string),
          ),
          (
            "<
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
>",
            [__words, __words]
            |> (Fmt.(root(box(string_set_set))) |> Fmt.to_to_string),
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
            [__long_words, __long_words]
            |> (Fmt.(root(box(string_set_set))) |> Fmt.to_to_string),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "array()"
    >: (
      () =>
        [
          ("[]", [] |> (Fmt.(root(array(string))) |> Fmt.to_to_string)),
          (
            "[a, b, c]",
            ["a", "b", "c"]
            |> (Fmt.(root(array(string))) |> Fmt.to_to_string),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "tuple()"
    >: (
      () =>
        [
          ("()", [] |> (Fmt.(root(tuple(string))) |> Fmt.to_to_string)),
          (
            "(a, b, c)",
            ["a", "b", "c"]
            |> (Fmt.(root(tuple(string))) |> Fmt.to_to_string),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "closure()"
    >: (
      () =>
        [
          ("{ }", [] |> (Fmt.(root(closure(string))) |> Fmt.to_to_string)),
          (
            "{
  foo
}",
            ["foo"] |> (Fmt.(root(closure(string))) |> Fmt.to_to_string),
          ),
          (
            "{
  foo
  bar
  fizz
  buzz
}",
            ["foo", "bar", "fizz", "buzz"]
            |> (Fmt.(root(closure(string))) |> Fmt.to_to_string),
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
            |> (Fmt.(root(closure(closure(string)))) |> Fmt.to_to_string),
          ),
          (
            "prefix {
  foo
}",
            ["foo"]
            |> (
              Fmt.(root(ppf => pf(ppf, "prefix %a", closure(string))))
              |> Fmt.to_to_string
            ),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "attribute()"
    >: (
      () =>
        [
          (
            "foo: bar",
            ("foo", "bar")
            |> (Fmt.(root(attribute(string, string))) |> Fmt.to_to_string),
          ),
          (
            "foo: Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
            (
              "foo",
              "Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
            )
            |> (Fmt.(root(attribute(string, string))) |> Fmt.to_to_string),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "record()"
    >: (
      () =>
        [
          (
            "{ }",
            [] |> (Fmt.(root(record(string, string))) |> Fmt.to_to_string),
          ),
          (
            "{
  foo: bar
}",
            [("foo", "bar")]
            |> (Fmt.(root(record(string, string))) |> Fmt.to_to_string),
          ),
          (
            "{
  foo: bar
  fizz: buzz
  zipp: zapp
}",
            [("foo", "bar"), ("fizz", "buzz"), ("zipp", "zapp")]
            |> (Fmt.(root(record(string, string))) |> Fmt.to_to_string),
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
            |> (Fmt.(root(record(string, string))) |> Fmt.to_to_string),
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
            |> (
              Fmt.(root(record(string, record(string, string))))
              |> Fmt.to_to_string
            ),
          ),
        ]
        |> Assert.(test_many(string))
    ),
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
    "entity()"
    >: (
      () =>
        [
          ("foo { }", ("foo", []) |> ~@Fmt.(entity(string, string))),
          (
            "foo {
  a
  b
  c
}",
            ("foo", ["a", "b", "c"]) |> ~@Fmt.(entity(string, string)),
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
            |> ~@Fmt.(entity(string, entity(string, string))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
    "struct_()"
    >: (
      () =>
        [
          ("foo { }", ("foo", []) |> ~@Fmt.(struct_(string, string))),
          (
            "foo {
  a: 1
  b: 2
  c: 3
}",
            ("foo", [("a", 1), ("b", 2), ("c", 3)])
            |> ~@Fmt.(struct_(string, int)),
          ),
          (
            "Hashtbl {
  foo: bar
}",
            ("Hashtbl", [("foo", "bar")])
            |> ~@Fmt.(struct_(string, string)),
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
            |> ~@Fmt.(struct_(string, struct_(string, int))),
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
