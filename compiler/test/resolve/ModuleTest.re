open Kore;
open Util;

module Module = Resolve.Module;

exception MockError;

let __program = [
  AST.(
    (
      "ABC" |> of_public |> Util.as_lexeme |> of_named_export,
      123
      |> Int64.of_int
      |> of_int
      |> of_num
      |> as_typed_lexeme(
           ~cursor=Cursor.range((4, 15), (4, 17)),
           Type.K_Strong(K_Integer),
         )
      |> of_prim
      |> as_int
      |> of_const,
    )
    |> of_decl
  ),
];

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
    "exists()"
    >: (
      () =>
        [
          (true, Module.exists(Module.Raw("foo"))),
          (
            false,
            Module.exists(
              Module.File({relative: "./bar", full: "/foo/bar"}),
            ),
          ),
          (
            true,
            Module.exists(
              Module.File({relative: "./bar", full: fixture_path}),
            ),
          ),
        ]
        |> Assert.(test_many(bool))
    ),
    "read() - raw"
    >: (
      () => {
        let content = "foo";

        let program =
          Module.read(
            stream => {
              Util.read_lazy_char_stream(stream) |> Assert.string(content);
              __program;
            },
            Module.Raw(content),
          )
          |> Result.get_ok;

        Assert.program(__program, program);
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
              __program;
            },
            Module.File({relative: "foo", full: fixture_path}),
          )
          |> Result.get_ok;

        Assert.program(__program, program);
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

        Filename.concat(cache, relative_path)
        |> Sys.file_exists
        |> Assert.true_;
        cached_path |> String.starts_with(cache) |> Assert.true_;
        cached_path |> String.ends_with(relative_path) |> Assert.true_;
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
  ];
