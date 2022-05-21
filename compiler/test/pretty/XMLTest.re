open Kore;
open Fmt;
open Pretty.XML;

module X = Fixtures.XML;

let _empty_node = name => Node(name, [], []);

let suite =
  "Pretty.XML"
  >::: [
    "empty tag"
    >: (() => Assert.string("<foo />", _empty_node("foo") |> ~@xml(string))),
    "with few attributes"
    >: (
      () =>
        Assert.string(
          "<foo fizz=buzz zip=zap />",
          X.with_few_attributes |> ~@xml(string),
        )
    ),
    "with many attributes"
    >: (
      () =>
        Assert.string(
          "<foo
  Ut=ex
  ex=veniam
  nostrud=dolor
  tempor=mollit
  id=commodo
  deserunt=pariatur
  ipsum=pariatur
/>",
          X.with_many_attributes |> ~@xml(string),
        )
    ),
    "with single child"
    >: (
      () =>
        Assert.string(
          "<foo>
  <bar />
</foo>",
          Node("foo", [], [_empty_node("bar")]) |> ~@xml(string),
        )
    ),
    "with many children"
    >: (
      () =>
        Assert.string(
          "<foo>
  <bar />
  <fizz />
  <buzz />
</foo>",
          Node(
            "foo",
            [],
            [_empty_node("bar"), _empty_node("fizz"), _empty_node("buzz")],
          )
          |> ~@xml(string),
        )
    ),
    "with few attributes and child tag"
    >: (
      () =>
        Assert.string(
          "<foo fizz=buzz zip=zap>
  <bar />
</foo>",
          Node("foo", X.few_attributes, [_empty_node("bar")])
          |> ~@xml(string),
        )
    ),
    "with child tag with few attributes"
    >: (
      () =>
        Assert.string(
          "<bar>
  <foo fizz=buzz zip=zap />
</bar>",
          Node("bar", [], [X.with_few_attributes]) |> ~@xml(string),
        )
    ),
    "with few attributes and child tag with few attributes"
    >: (
      () =>
        Assert.string(
          "<bar fizz=buzz zip=zap>
  <foo fizz=buzz zip=zap />
</bar>",
          Node("bar", X.few_attributes, [X.with_few_attributes])
          |> ~@xml(string),
        )
    ),
    "with many attributes and child tag with few attributes"
    >: (
      () =>
        Assert.string(
          "<foo
  Ut=ex
  ex=veniam
  nostrud=dolor
  tempor=mollit
  id=commodo
  deserunt=pariatur
  ipsum=pariatur
>
  <bar fizz=buzz zip=zap />
</foo>",
          Node(
            "foo",
            X.many_attributes,
            [Node("bar", X.few_attributes, [])],
          )
          |> ~@xml(string),
        )
    ),
    "with few attributes and child tag with many attributes"
    >: (
      () =>
        Assert.string(
          "<bar fizz=buzz zip=zap>
  <foo
    Ut=ex
    ex=veniam
    nostrud=dolor
    tempor=mollit
    id=commodo
    deserunt=pariatur
    ipsum=pariatur
  />
</bar>",
          Node("bar", X.few_attributes, [X.with_many_attributes])
          |> ~@xml(string),
        )
    ),
    "with many attributes and child tag with many attributes"
    >: (
      () =>
        Assert.string(
          "<bar
  Ut=ex
  ex=veniam
  nostrud=dolor
  tempor=mollit
  id=commodo
  deserunt=pariatur
  ipsum=pariatur
>
  <foo
    Ut=ex
    ex=veniam
    nostrud=dolor
    tempor=mollit
    id=commodo
    deserunt=pariatur
    ipsum=pariatur
  />
</bar>",
          Node("bar", X.many_attributes, [X.with_many_attributes])
          |> ~@xml(string),
        )
    ),
  ];
