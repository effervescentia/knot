open Kore;
open Fmt;
open Pretty.XML;

let suite =
  "Pretty.XML"
  >::: [
    "empty tag"
    >: (
      () => Assert.string("<foo />", Node("foo", [], []) |> ~@xml(string))
    ),
    "with few attributes"
    >: (
      () =>
        Assert.string(
          "<foo fizz=buzz zip=zap />",
          Node("foo", [("fizz", "buzz"), ("zip", "zap")], [])
          |> ~@xml(string),
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
          Node(
            "foo",
            [
              ("Ut", "ex"),
              ("ex", "veniam"),
              ("nostrud", "dolor"),
              ("tempor", "mollit"),
              ("id", "commodo"),
              ("deserunt", "pariatur"),
              ("ipsum", "pariatur"),
            ],
            [],
          )
          |> ~@xml(string),
        )
    ),
    "with single child"
    >: (
      () =>
        Assert.string(
          "<foo>
  <bar />
</foo>",
          Node("foo", [], [Node("bar", [], [])]) |> ~@xml(string),
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
            [
              Node("bar", [], []),
              Node("fizz", [], []),
              Node("buzz", [], []),
            ],
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
          Node(
            "foo",
            [("fizz", "buzz"), ("zip", "zap")],
            [Node("bar", [], [])],
          )
          |> ~@xml(string),
        )
    ),
    "with child tag with few attributes"
    >: (
      () =>
        Assert.string(
          "<foo>
  <bar fizz=buzz zip=zap />
</foo>",
          Node(
            "foo",
            [],
            [Node("bar", [("fizz", "buzz"), ("zip", "zap")], [])],
          )
          |> ~@xml(string),
        )
    ),
    "with few attributes and child tag with few attributes"
    >: (
      () =>
        Assert.string(
          "<foo fizz=buzz zip=zap>
  <bar fizz=buzz zip=zap />
</foo>",
          Node(
            "foo",
            [("fizz", "buzz"), ("zip", "zap")],
            [Node("bar", [("fizz", "buzz"), ("zip", "zap")], [])],
          )
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
            [
              ("Ut", "ex"),
              ("ex", "veniam"),
              ("nostrud", "dolor"),
              ("tempor", "mollit"),
              ("id", "commodo"),
              ("deserunt", "pariatur"),
              ("ipsum", "pariatur"),
            ],
            [Node("bar", [("fizz", "buzz"), ("zip", "zap")], [])],
          )
          |> ~@xml(string),
        )
    ),
    "with few attributes and child tag with many attributes"
    >: (
      () =>
        Assert.string(
          "<foo fizz=buzz zip=zap>
  <bar
    Ut=ex
    ex=veniam
    nostrud=dolor
    tempor=mollit
    id=commodo
    deserunt=pariatur
    ipsum=pariatur
  />
</foo>",
          Node(
            "foo",
            [("fizz", "buzz"), ("zip", "zap")],
            [
              Node(
                "bar",
                [
                  ("Ut", "ex"),
                  ("ex", "veniam"),
                  ("nostrud", "dolor"),
                  ("tempor", "mollit"),
                  ("id", "commodo"),
                  ("deserunt", "pariatur"),
                  ("ipsum", "pariatur"),
                ],
                [],
              ),
            ],
          )
          |> ~@xml(string),
        )
    ),
    "with many attributes and child tag with many attributes"
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
  <bar
    Ut=ex
    ex=veniam
    nostrud=dolor
    tempor=mollit
    id=commodo
    deserunt=pariatur
    ipsum=pariatur
  />
</foo>",
          Node(
            "foo",
            [
              ("Ut", "ex"),
              ("ex", "veniam"),
              ("nostrud", "dolor"),
              ("tempor", "mollit"),
              ("id", "commodo"),
              ("deserunt", "pariatur"),
              ("ipsum", "pariatur"),
            ],
            [
              Node(
                "bar",
                [
                  ("Ut", "ex"),
                  ("ex", "veniam"),
                  ("nostrud", "dolor"),
                  ("tempor", "mollit"),
                  ("id", "commodo"),
                  ("deserunt", "pariatur"),
                  ("ipsum", "pariatur"),
                ],
                [],
              ),
            ],
          )
          |> ~@xml(string),
        )
    ),
  ];
