open Kore;
open Fmt;
open Pretty.XML;

let suite =
  "Pretty.XML"
  >::: [
    "xml()"
    >: (
      () =>
        [
          ("<foo />", Parent("foo", [], []) |> ~@xml),
          ("<foo />", Child("foo", []) |> ~@xml),
          (
            "<foo fizz=buzz zip=zap />",
            Parent("foo", [("fizz", "buzz"), ("zip", "zap")], []) |> ~@xml,
          ),
          (
            "<foo fizz=buzz zip=zap />",
            Child("foo", [("fizz", "buzz"), ("zip", "zap")]) |> ~@xml,
          ),
          /* Ut ex ex veniam nostrud dolor tempor mollit id commodo deserunt pariatur ipsum pariatur. */
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
            Parent(
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
            |> ~@xml,
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
            Child(
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
            )
            |> ~@xml,
          ),
          (
            "<foo>
  <bar />
</foo>",
            Parent("foo", [], [Child("bar", [])]) |> ~@xml,
          ),
          (
            "<foo fizz=buzz zip=zap>
  <bar />
</foo>",
            Parent(
              "foo",
              [("fizz", "buzz"), ("zip", "zap")],
              [Child("bar", [])],
            )
            |> ~@xml,
          ),
          (
            "<foo>
  <bar fizz=buzz zip=zap />
</foo>",
            Parent(
              "foo",
              [],
              [Child("bar", [("fizz", "buzz"), ("zip", "zap")])],
            )
            |> ~@xml,
          ),
          (
            "<foo fizz=buzz zip=zap>
  <bar fizz=buzz zip=zap />
</foo>",
            Parent(
              "foo",
              [("fizz", "buzz"), ("zip", "zap")],
              [Child("bar", [("fizz", "buzz"), ("zip", "zap")])],
            )
            |> ~@xml,
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
            Parent(
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
              [Child("bar", [("fizz", "buzz"), ("zip", "zap")])],
            )
            |> ~@xml,
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
            Parent(
              "foo",
              [("fizz", "buzz"), ("zip", "zap")],
              [
                Child(
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
                ),
              ],
            )
            |> ~@xml,
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
            Parent(
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
                Child(
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
                ),
              ],
            )
            |> ~@xml,
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
