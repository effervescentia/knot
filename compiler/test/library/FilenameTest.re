open Kore;

let suite =
  "Library.Filename"
  >::: [
    "normalize()"
    >: (
      () => {
        Assert.string("foo", Filename.normalize("foo"));
        Assert.string("foo/bar", Filename.normalize("foo/bar"));
      }
    ),
    "normalize() - simplify path separators"
    >: (
      () => {
        Assert.string("/", Filename.normalize("/"));
        Assert.string("/", Filename.normalize("///"));
        Assert.string("/foo", Filename.normalize("/foo"));
        Assert.string("/foo", Filename.normalize("///foo///"));
      }
    ),
    "normalize() - resolve special directories"
    >: (
      () => {
        Assert.string("", Filename.normalize("."));
        Assert.string("", Filename.normalize("./"));
        Assert.string("", Filename.normalize("././"));
        Assert.string("foo/bar", Filename.normalize("foo/./bar"));
        Assert.string("..", Filename.normalize(".."));
        Assert.string("..", Filename.normalize("../"));
        Assert.string("..", Filename.normalize("./../."));
        Assert.string("bar", Filename.normalize("foo/../bar"));
        Assert.string("foo/fizz", Filename.normalize("foo/bar/../fizz"));
        Assert.string("../bar", Filename.normalize("foo/../../bar"));
        Assert.string("../bar", Filename.normalize("../foo/../bar"));
        Assert.string("../../bar", Filename.normalize("../../foo/../bar"));
      }
    ),
    "concat() - empty path"
    >: (
      () => {
        Assert.string("", Filename.concat("", ""));
        Assert.string("", Filename.concat("", "."));
        Assert.string("", Filename.concat("", "./"));
        Assert.string("", Filename.concat(".", "./"));
        Assert.string("", Filename.concat(".", "."));
      }
    ),
    "concat() - root dir"
    >: (
      () => {
        Assert.string("/", Filename.concat("/", ""));
        Assert.string("/", Filename.concat("/", "."));
        Assert.string("/", Filename.concat("/", "./"));
        Assert.string("/", Filename.concat("/.", "./"));
        Assert.string("/", Filename.concat("/.", "."));
      }
    ),
    "concat() - relative path"
    >: (
      () => {
        Assert.string("x", Filename.concat("x", ""));
        Assert.string("x", Filename.concat("", "x"));
        Assert.string("x", Filename.concat("", "x/"));
        Assert.string("x/x", Filename.concat("x", "x"));
        Assert.string("x/x", Filename.concat("x", "x/"));
        Assert.string("x/x", Filename.concat("x/", "x/"));
      }
    ),
    "concat() - absolute path"
    >: (
      () => {
        Assert.string("/x/x", Filename.concat("/x", "x"));
        Assert.string("/x/x", Filename.concat("/x/", "x/"));
      }
    ),
    "resolve()"
    >: (
      () => {
        let cwd = Sys.getcwd();

        Assert.string(cwd, Filename.resolve(""));
        Assert.string(cwd, Filename.resolve("."));
        Assert.string(Filename.dirname(cwd), Filename.resolve(".."));
        Assert.string(cwd ++ "/foo", Filename.resolve("foo"));
        Assert.string(cwd ++ "/foo", Filename.resolve("./foo"));
        Assert.string("/foo", Filename.resolve("/foo"));
      }
    ),
    "relative_to()"
    >: (
      () => {
        Assert.string("../bar", Filename.relative_to("foo", "bar"));
        Assert.string(
          "../bar",
          Filename.relative_to(Filename.concat(Sys.getcwd(), "foo"), "bar"),
        );
        Assert.string(
          "../bar",
          Filename.relative_to("foo", Filename.concat(Sys.getcwd(), "bar")),
        );
        Assert.string(
          "../bar",
          Filename.relative_to("/foo/bizz", "/foo/bar"),
        );
      }
    ),
    "relative_to() - unrelated ancestors"
    >: (
      () => {
        Assert.string("../bar", Filename.relative_to("/foo", "/bar"));
        Assert.string("../a/b/c", Filename.relative_to("/1", "/a/b/c"));
        Assert.string("../../../a", Filename.relative_to("/1/2/3", "/a"));
        Assert.string(
          "../../../a/b/c",
          Filename.relative_to("/1/2/3", "/a/b/c"),
        );
      }
    ),
  ];
