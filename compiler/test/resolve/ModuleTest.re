open Kore;

module Module = Resolve.Module;
module U = Util.ResultUtil;

exception MockError;

let suite =
  "Resolve.Module"
  >::: [
    "of_string()"
    >: (
      () => {
        let content = "foo";

        Assert.module_(Module.Raw(content), Module.of_string(content));
      }
    ),
    "of_file()"
    >: (
      () => {
        let path = Module.{relative: "./bar", full: "/foo/bar"};

        Assert.module_(Module.File(path), Module.of_file(path));
      }
    ),
    "exists() - raw module does exist"
    >: (() => Assert.true_(Module.exists(Module.Raw("foo")))),
    "exists() - file module does not exist"
    >: (
      () =>
        Assert.false_(
          Module.exists(Module.File({relative: "./bar", full: "/foo/bar"})),
        )
    ),
    "exists() - file module does exist"
    >: (
      () =>
        Assert.true_(
          Module.exists(
            Module.File({relative: "./bar", full: fixture_path}),
          ),
        )
    ),
    "read() - raw"
    >: (
      () => {
        let content = "foo";

        let program =
          Module.read(
            stream => {
              Assert.string(content, Util.read_lazy_char_stream(stream));
              Fixtures.Program.const_int;
            },
            Module.Raw(content),
          )
          |> Result.get_ok;

        Assert.program(Fixtures.Program.const_int, program);
      }
    ),
    "read() - file exists"
    >: (
      () => {
        let program =
          Module.read(
            stream => {
              Assert.string(
                "hello world\n",
                Util.read_lazy_char_stream(stream),
              );
              Fixtures.Program.const_int;
            },
            Module.File({relative: "foo", full: fixture_path}),
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
          Module.read(_ => [], Module.File({relative, full: "bar"}))
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
          Module.cache(
            cache,
            File({relative: relative_path, full: fixture_path}),
          )
          |> Result.get_ok;

        Assert.true_(
          Sys.file_exists(Filename.concat(cache, relative_path)),
        );
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
          Module.cache(cache, File({relative: relative_path, full: "bar"}))
          |> Result.get_error,
        );
      }
    ),
    "pp() - raw"
    >: (
      () =>
        Assert.string(
          "Module {
  raw: foo
}",
          Module.Raw("foo") |> ~@Fmt.root(Module.pp),
        )
    ),
    "pp() - file"
    >: (
      () =>
        Assert.string(
          "Module {
  full: foo
  relative: bar
}",
          Module.File({full: "foo", relative: "bar"})
          |> ~@Fmt.root(Module.pp),
        )
    ),
  ];
