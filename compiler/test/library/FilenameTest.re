open Kore;

let suite =
  "Library - Filename"
  >::: [
    "normalize"
    >: (
      () => {
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
        |> Assert.test_many(Assert.string);
      }
    ),
    "concat"
    >: (
      () => {
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
          ("x", Filename.concat("", "x")),
          ("x", Filename.concat("", "x/")),
          ("x/x", Filename.concat("x", "x")),
          ("x/x", Filename.concat("x", "x/")),
          ("x/x", Filename.concat("x/", "x/")),
          ("/x/x", Filename.concat("/x", "x")),
          ("/x/x", Filename.concat("/x/", "x/")),
        ]
        |> Assert.test_many(Assert.string);
      }
    ),
  ];
