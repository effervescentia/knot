open Kore;

module Module = Resolve.Module;

exception MockError;

let __program = [
  AST.(
    of_decl((
      "ABC" |> of_public |> Util.as_lexeme,
      (
        123 |> Int64.of_int |> of_int |> of_num,
        Type.K_Integer,
        Cursor.range((4, 15), (4, 17)),
      )
      |> of_prim
      |> of_const,
    ))
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
          );

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
          );

        Assert.program(__program, program);
      }
    ),
    "read() - file does not exist"
    >: (
      () => {
        let relative = "foo.kn";

        Alcotest.check_raises(
          "should throw FileNotFound exception",
          CompilerError([FileNotFound(relative)]),
          () =>
          Module.read(_ => [], Module.File({relative, full: "bar"}))
          |> ignore
        );
      }
    ),
    "cache() - file exists"
    >: (
      () => {
        let relative_path = "foo.txt";
        let cache = Util.get_temp_dir();

        Module.cache(
          cache,
          File({relative: relative_path, full: fixture_path}),
        );

        Filename.concat(cache, relative_path)
        |> Sys.file_exists
        |> Assert.true_;
      }
    ),
    "cache() - file does not exist"
    >: (
      () => {
        let relative_path = "foo.txt";
        let cache = Util.get_temp_dir();

        Alcotest.check_raises(
          "should throw FileNotFound exception",
          CompilerError([FileNotFound(relative_path)]),
          () =>
          Module.cache(cache, File({relative: relative_path, full: "bar"}))
        );
      }
    ),
  ];
