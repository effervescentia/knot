open Kore;

module Module = Resolve.Module;

let _range = ((l, l'), (r, r')) =>
  Cursor.range(Cursor.point(l, l'), Cursor.point(r, r'));

let __program = [
  AST.(
    of_decl((
      "ABC",
      of_const(
        of_prim(
          Block.create(
            ~type_=Type.K_Integer,
            _range((4, 15), (4, 17)),
            of_num(
              Block.create(
                ~type_=Type.K_Integer,
                _range((4, 15), (4, 17)),
                of_int(Int64.of_int(123)),
              ),
            ),
          ),
        ),
      ),
    ))
  ),
];

let suite =
  "Resolve.Module"
  >::: [
    "of_string()"
    >: (
      () => {
        let contents = "foo";

        Assert.module_(Module.Raw(contents), Module.of_string(contents));
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
        let contents = "foo";

        let program =
          Module.read(
            stream => {
              Util.read_lazy_char_stream(stream) |> Assert.string(contents);
              __program;
            },
            Module.Raw(contents),
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
                "hello world",
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
      () =>
        Alcotest.check_raises(
          "should throw UnresolvedModule exception",
          CompilerError(UnresolvedModule("foo")),
          () =>
          Module.read(_ => [], Module.File({relative: "foo", full: "bar"}))
          |> ignore
        )
    ),
    "read() - parse invalid program"
    >: (
      () =>
        Module.read(
          stream => Assert.fail("force error"),
          Module.File({relative: "foo", full: fixture_path}),
        )
        |> Assert.program([])
    ),
    "cache()"
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
  ];
