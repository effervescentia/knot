open Kore;
open Fmt;
open Pretty.XML;

/* let _repack_node: node_t('a, 'b) => node_t('a, 'b) = x => x; */

let suite =
  "Pretty.XML"
  >::: [
    "xml ()"
    >: (
      () =>
        [
          ("<foo />", Node("foo", [], []) |> ~@xml(string)),
          (
            "<foo fizz=buzz zip=zap />",
            Node("foo", [("fizz", "buzz"), ("zip", "zap")], [])
            |> ~@xml(string),
          ),
          (
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
          ),
          (
            "<foo>
  <bar />
</foo>",
            Node("foo", [], [Node("bar", [], [])]) |> ~@xml(string),
          ),
          (
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
          ),
          (
            "<foo fizz=buzz zip=zap>
  <bar />
</foo>",
            Node(
              "foo",
              [("fizz", "buzz"), ("zip", "zap")],
              [Node("bar", [], [])],
            )
            |> ~@xml(string),
          ),
          (
            "<foo>
  <bar fizz=buzz zip=zap />
</foo>",
            Node(
              "foo",
              [],
              [Node("bar", [("fizz", "buzz"), ("zip", "zap")], [])],
            )
            |> ~@xml(string),
          ),
          (
            "<foo fizz=buzz zip=zap>
  <bar fizz=buzz zip=zap />
</foo>",
            Node(
              "foo",
              [("fizz", "buzz"), ("zip", "zap")],
              [Node("bar", [("fizz", "buzz"), ("zip", "zap")], [])],
            )
            |> ~@xml(string),
          ),
          (
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
          ),
          (
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
          ),
          (
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
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
