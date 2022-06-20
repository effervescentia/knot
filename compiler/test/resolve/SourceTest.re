open Kore;

module Source = Resolve.Source;
module U = Util.ResultUtil;

exception MockError;

let suite =
  "Resolve.Source"
  >::: [
    "of_string()"
    >: (
      () => {
        let content = "foo";

        Assert.source(Source.Raw(content), Source.of_string(content));
      }
    ),
    "of_file()"
    >: (
      () => {
        let path = Source.{relative: "./bar", full: "/foo/bar"};

        Assert.source(Source.File(path), Source.of_file(path));
      }
    ),
    "exists() - raw module does exist"
    >: (() => Assert.true_(Source.exists(Source.Raw("foo")))),
    "exists() - file module does not exist"
    >: (
      () =>
        Assert.false_(
          Source.exists(Source.File({relative: "./bar", full: "/foo/bar"})),
        )
    ),
    "exists() - file module does exist"
    >: (
      () =>
        Assert.true_(
          Source.exists(
            Source.File({relative: "./bar", full: fixture_path}),
          ),
        )
    ),
    "read() - raw"
    >: (
      () => {
        let content = "foo";

        let program =
          Source.read(
            stream => {
              Assert.string(content, Util.read_lazy_char_stream(stream));
              Fixtures.Program.const_int;
            },
            Source.Raw(content),
          )
          |> Result.get_ok;

        Assert.program(Fixtures.Program.const_int, program);
      }
    ),
    "read() - file exists"
    >: (
      () => {
        let program =
          Source.read(
            stream => {
              Assert.string(
                "hello world\n",
                Util.read_lazy_char_stream(stream),
              );
              Fixtures.Program.const_int;
            },
            Source.File({relative: "foo", full: fixture_path}),
          )
          |> Result.get_ok;

        Assert.program(Fixtures.Program.const_int, program);
      }
    ),
    "read() - file does not exist"
    >: (
      () => {
        let relative = "foo.kn";

        Assert.compile_errors(
          [FileNotFound(relative)],
          Source.read(_ => [], Source.File({relative, full: "bar"}))
          |> Result.get_error,
        );
      }
    ),
    "cache() - file exists"
    >: (
      () => {
        let relative_path = "foo.txt";
        let cache = Util.get_temp_dir();

        let cached_path =
          Source.cache(
            cache,
            File({relative: relative_path, full: fixture_path}),
          )
          |> Result.get_ok;

        Assert.file_exists(Filename.concat(cache, relative_path));
        Assert.true_(String.starts_with(cache, cached_path));
        Assert.true_(String.ends_with(relative_path, cached_path));
      }
    ),
    "cache() - file does not exist"
    >: (
      () => {
        let relative_path = "foo.txt";
        let cache = Util.get_temp_dir();

        Assert.compile_errors(
          [FileNotFound(relative_path)],
          Source.cache(cache, File({relative: relative_path, full: "bar"}))
          |> Result.get_error,
        );
      }
    ),
    "pp() - raw"
    >: (
      () =>
        Assert.string(
          "Source {
  raw: foo
}",
          Source.Raw("foo") |> ~@Fmt.root(Source.pp),
        )
    ),
    "pp() - file"
    >: (
      () =>
        Assert.string(
          "Source {
  full: foo
  relative: bar
}",
          Source.File({full: "foo", relative: "bar"})
          |> ~@Fmt.root(Source.pp),
        )
    ),
  ];
