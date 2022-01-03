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

let _set = x => box(collection(any("<"), any(">"), x));
let _string_set = _set(string);
let _string_set_set = _set(_string_set);

let suite =
  "Pretty.Container"
  >::: [
    "root()"
    >: (() => Assert.string("<foo></foo>", "<foo></foo>" |> ~@root(string))),
    "page()"
    >: (() => Assert.string("<foo></foo>
", "<foo></foo>" |> ~@page(string))),
    "list() - empty"
    >: (() => Assert.string("", [] |> ~@root(list(string)))),
    "list() - comma separated"
    >: (
      () =>
        Assert.string(
          "a, b, c",
          ["a", "b", "c"] |> ~@root(box(list(string))),
        )
    ),
    "list() - trailing comma"
    >: (
      () =>
        Assert.string("a, b, c,
", ["a", "b", "c"] |> ~@root(list(string)))
    ),
    "list() - semicolon separated"
    >: (
      () =>
        Assert.string(
          "a; b; c",
          ["a", "b", "c"]
          |> ~@root(box(list(~sep=Sep.of_sep(";"), string))),
        )
    ),
    "list() - do not break line when content fits"
    >: (
      () =>
        Assert.string(
          "Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi",
          __words |> ~@root(box(list(string))),
        )
    ),
    "list() - break line when too long"
    >: (
      () =>
        Assert.string(
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
        )
    ),
    "collection() - empty"
    >: (() => Assert.string("<>", [] |> ~@root(_string_set))),
    "collection() - comma separated"
    >: (
      () =>
        Assert.string("<a, b, c>", ["a", "b", "c"] |> ~@root(_string_set))
    ),
    "collection() - do not break line when content fits"
    >: (
      () =>
        Assert.string(
          "<Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>",
          __words |> ~@root(_string_set),
        )
    ),
    "collection() - break line when too long"
    >: (
      () =>
        Assert.string(
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
          __long_words |> ~@root(_string_set),
        )
    ),
    "collection() - nested inline"
    >: (
      () =>
        Assert.string(
          "<<a, b, c>, <d, e, f>>",
          [["a", "b", "c"], ["d", "e", "f"]] |> ~@root(_string_set_set),
        )
    ),
    "collection() - nested break line"
    >: (
      () =>
        Assert.string(
          "<
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
  <Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>,
>",
          [__words, __words] |> ~@root(box(_string_set_set)),
        )
    ),
    "collection() - nested breaking all lines"
    >: (
      () =>
        Assert.string(
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
          [__long_words, __long_words] |> ~@root(box(_string_set_set)),
        )
    ),
    "array() - empty"
    >: (() => Assert.string("[]", [] |> ~@root(array(string)))),
    "array() - not empty"
    >: (
      () =>
        Assert.string(
          "[a, b, c]",
          ["a", "b", "c"] |> ~@root(array(string)),
        )
    ),
    "tuple() - empty"
    >: (() => Assert.string("()", [] |> ~@root(tuple(string)))),
    "tuple() - not empty"
    >: (
      () =>
        Assert.string(
          "(a, b, c)",
          ["a", "b", "c"] |> ~@root(tuple(string)),
        )
    ),
    "closure() - empty"
    >: (() => Assert.string("{ }", [] |> ~@root(closure(string)))),
    "closure() - one entry"
    >: (
      () => Assert.string("{
  foo
}", ["foo"] |> ~@root(closure(string)))
    ),
    "closure() - many entries"
    >: (
      () =>
        Assert.string(
          "{
  foo
  bar
  fizz
  buzz
}",
          ["foo", "bar", "fizz", "buzz"] |> ~@root(closure(string)),
        )
    ),
    "closure() - nested"
    >: (
      () =>
        Assert.string(
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
        )
    ),
    "closure() - prefixed"
    >: (
      () =>
        Assert.string(
          "prefix {
  foo
}",
          ["foo"] |> ~@root(ppf => pf(ppf, "prefix %a", closure(string))),
        )
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
