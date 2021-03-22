open Kore;

module Format = Executable.Format;

let __compiler_config =
  Compile.Compiler.{
    name: "foo",
    entry: Internal("main"),
    root_dir: "placeholder",
    source_dir: ".",
  };

let suite =
  "Executable.Format"
  >::: [
    "run() - no errors"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let output_file = Filename.concat(temp_dir, "main.kn");

        FileUtil.cp(~recurse=true, [messy_fixture_dir], temp_dir);

        Format.run(
          ~catch=print_errs % Assert.fail,
          {...__compiler_config, root_dir: temp_dir},
          (),
        );

        output_file |> Sys.file_exists |> Assert.true_;
        output_file
        |> Util.read_file_to_string
        |> Assert.string("const ABC = 123;\n");
      }
    ),
    "run() - ignore cyclic errors"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let main_file = Filename.concat(temp_dir, "main.kn");
        let utils_file = Filename.concat(temp_dir, "utils.kn");

        FileUtil.cp(~recurse=true, [messy_cylic_fixture_dir], temp_dir);

        Format.run(
          ~catch=print_errs % Assert.fail,
          {...__compiler_config, root_dir: temp_dir},
          (),
        );

        main_file |> Sys.file_exists |> Assert.true_;
        main_file
        |> Util.read_file_to_string
        |> Assert.string("import Utils from \"@/utils\";\n");

        utils_file |> Sys.file_exists |> Assert.true_;
        utils_file
        |> Util.read_file_to_string
        |> Assert.string("import Main from \"@/main\";\n");
      }
    ),
    "run() - ignore missing files"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let main_file = Filename.concat(temp_dir, "main.kn");

        FileUtil.cp(
          ~recurse=true,
          [messy_missing_import_fixture_dir],
          temp_dir,
        );

        Format.run(
          ~catch=print_errs % Assert.fail,
          {...__compiler_config, root_dir: temp_dir},
          (),
        );

        main_file |> Sys.file_exists |> Assert.true_;
        main_file
        |> Util.read_file_to_string
        |> Assert.string("import Utils from \"@/utils\";\n");
      }
    ),
  ];
