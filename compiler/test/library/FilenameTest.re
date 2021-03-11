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
          ("bar", Filename.normalize("foo/../bar")),
          ("foo/fizz", Filename.normalize("foo/bar/../fizz")),
          ("../bar", Filename.normalize("foo/../../bar")),
          ("../bar", Filename.normalize("../foo/../bar")),
          ("../../bar", Filename.normalize("../../foo/../bar")),
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
          (Filename.dirname(cwd), Filename.resolve("..")),
          (cwd ++ "/foo", Filename.resolve("foo")),
          (cwd ++ "/foo", Filename.resolve("./foo")),
          ("/foo", Filename.resolve("/foo")),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "relative_to()"
    >: (
      () => {
        [
          ("../bar", Filename.relative_to("foo", "bar")),
          (
            "../bar",
            Filename.relative_to(
              Filename.concat(Sys.getcwd(), "foo"),
              "bar",
            ),
          ),
          (
            "../bar",
            Filename.relative_to(
              "foo",
              Filename.concat(Sys.getcwd(), "bar"),
            ),
          ),
          ("../bar", Filename.relative_to("/foo", "/bar")),
          ("../bar", Filename.relative_to("/foo/bizz", "/foo/bar")),
          ("../a/b/c", Filename.relative_to("/1", "/a/b/c")),
          ("../../../a", Filename.relative_to("/1/2/3", "/a")),
          ("../../../a/b/c", Filename.relative_to("/1/2/3", "/a/b/c")),
        ]
        |> Assert.(test_many(string));
      }
    ),
  ];
