open Kore;

let suite =
  "Library.Filename"
  >::: [
    "normalize()"
    >: (
      () =>
        [
          ("/", Filename.normalize("/")),
          ("/", Filename.normalize("///")),
          ("/foo", Filename.normalize("/foo")),
          ("/foo", Filename.normalize("///foo///")),
          ("", Filename.normalize(".")),
          ("", Filename.normalize("./")),
          ("", Filename.normalize("././")),
          ("foo", Filename.normalize("foo")),
          ("foo/bar", Filename.normalize("foo/bar")),
          ("foo/bar", Filename.normalize("foo/./bar")),
          ("..", Filename.normalize("..")),
          ("..", Filename.normalize("../")),
          ("..", Filename.normalize("./../.")),
        ]
        |> Assert.(test_many(string))
    ),
    "concat()"
    >: (
      () =>
        [
          ("", Filename.concat("", "")),
          ("", Filename.concat("", ".")),
          ("", Filename.concat("", "./")),
          ("", Filename.concat(".", "./")),
          ("", Filename.concat(".", ".")),
          ("/", Filename.concat("/", "")),
          ("/", Filename.concat("/", ".")),
          ("/", Filename.concat("/", "./")),
          ("/", Filename.concat("/.", "./")),
          ("/", Filename.concat("/.", ".")),
          ("x", Filename.concat("x", "")),
          ("x", Filename.concat("", "x")),
          ("x", Filename.concat("", "x/")),
          ("x/x", Filename.concat("x", "x")),
          ("x/x", Filename.concat("x", "x/")),
          ("x/x", Filename.concat("x/", "x/")),
          ("/x/x", Filename.concat("/x", "x")),
          ("/x/x", Filename.concat("/x/", "x/")),
        ]
        |> Assert.(test_many(string))
    ),
    "resolve()"
    >: (
      () => {
        let cwd = Sys.getcwd();

        [
          (cwd, Filename.resolve("")),
          (cwd, Filename.resolve(".")),
          (cwd ++ "/..", Filename.resolve("..")),
          (cwd ++ "/foo", Filename.resolve("foo")),
          (cwd ++ "/foo", Filename.resolve("./foo")),
          ("/foo", Filename.resolve("/foo")),
        ]
        |> Assert.(test_many(string));
      }
    ),
  ];
