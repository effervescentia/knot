open Kore;
open Fmt;
open Pretty.Container;

module L = Fixtures.List;

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
    >: (() => Assert.string("a, b, c", L.abc |> ~@root(box(list(string))))),
    "list() - trailing comma"
    >: (() => Assert.string("a, b, c,
", L.abc |> ~@root(list(string)))),
    "list() - semicolon separated"
    >: (
      () =>
        Assert.string(
          "a; b; c",
          L.abc |> ~@root(box(list(~sep=Sep.of_sep(";"), string))),
        )
    ),
    "list() - do not break line when content fits"
    >: (
      () =>
        Assert.string(
          "Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi",
          L.many |> ~@root(box(list(string))),
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
          L.too_many |> ~@root(list(string)),
        )
    ),
    "collection() - empty"
    >: (() => Assert.string("<>", [] |> ~@root(_string_set))),
    "collection() - comma separated"
    >: (() => Assert.string("<a, b, c>", L.abc |> ~@root(_string_set))),
    "collection() - do not break line when content fits"
    >: (
      () =>
        Assert.string(
          "<Sint, eiusmod, quis, consectetur, cillum, nulla, est, et, ipsum, nisi>",
          L.many |> ~@root(_string_set),
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
          L.too_many |> ~@root(_string_set),
        )
    ),
    "collection() - nested inline"
    >: (
      () =>
        Assert.string(
          "<<a, b, c>, <d, e, f>>",
          [L.abc, L.def] |> ~@root(_string_set_set),
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
          [L.many, L.many] |> ~@root(box(_string_set_set)),
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
          [L.too_many, L.too_many] |> ~@root(box(_string_set_set)),
        )
    ),
    "array() - empty"
    >: (() => Assert.string("[]", [] |> ~@root(array(string)))),
    "array() - not empty"
    >: (() => Assert.string("[a, b, c]", L.abc |> ~@root(array(string)))),
    "tuple() - empty"
    >: (() => Assert.string("()", [] |> ~@root(tuple(string)))),
    "tuple() - not empty"
    >: (() => Assert.string("(a, b, c)", L.abc |> ~@root(tuple(string)))),
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
  Sint
  eiusmod
  quis
  consectetur
  cillum
  nulla
  est
  et
  ipsum
  nisi
}",
          L.many |> ~@root(closure(string)),
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
    "attribute() - short value"
    >: (
      () =>
        Assert.string(
          "foo: bar",
          ("foo", "bar") |> ~@root(attribute(string, string)),
        )
    ),
    "attribute() - long value"
    >: (
      () =>
        Assert.string(
          "foo: Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
          (
            "foo",
            "Laboris pariatur nisi nulla dolor aliqua est voluptate fugiat est commodo",
          )
          |> ~@root(attribute(string, string)),
        )
    ),
    "record() - empty"
    >: (() => Assert.string("{ }", [] |> ~@root(record(string, string)))),
    "record() - one entry"
    >: (
      () =>
        Assert.string(
          "{
  foo: bar
}",
          [("foo", "bar")] |> ~@root(record(string, string)),
        )
    ),
    "record() - multiple entries"
    >: (
      () =>
        Assert.string(
          "{
  a: 1
  b: 2
  c: 3
}",
          L.a1b2c3 |> ~@root(record(string, int)),
        )
    ),
    "record() - long value"
    >: (
      () =>
        Assert.string(
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
        )
    ),
    "record() - nested"
    >: (
      () =>
        Assert.string(
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
        )
    ),
    "entity() - empty"
    >: (
      () =>
        Assert.string(
          "foo { }",
          ("foo", []) |> ~@root(entity(string, string)),
        )
    ),
    "entity() - not empty"
    >: (
      () =>
        Assert.string(
          "foo {
  a
  b
  c
}",
          ("foo", L.abc) |> ~@root(entity(string, string)),
        )
    ),
    "entity() - nested"
    >: (
      () =>
        Assert.string(
          "foo {
  bar {
    a
    b
    c
  }
  fizz {
    d
    e
    f
  }
}",
          ("foo", [("bar", L.abc), ("fizz", L.def)])
          |> ~@root(entity(string, entity(string, string))),
        )
    ),
    "struct_() - empty"
    >: (
      () =>
        Assert.string(
          "foo { }",
          ("foo", []) |> ~@root(struct_(string, string)),
        )
    ),
    "struct_() - not empty"
    >: (
      () =>
        Assert.string(
          "foo {
  a: 1
  b: 2
  c: 3
}",
          ("foo", L.a1b2c3) |> ~@root(struct_(string, int)),
        )
    ),
    "struct_() - nested"
    >: (
      () =>
        Assert.string(
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
            [("bar", ("Bar", L.a1b2c3)), ("fizz", ("Fizz", L.d4e5f6))],
          )
          |> ~@root(struct_(string, struct_(string, int))),
        )
    ),
  ];
