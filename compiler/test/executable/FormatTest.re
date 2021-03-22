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
    >~:: (
      (_, ()) => {
        let temp_dir = Util.get_temp_dir();
        let output_file = Filename.concat(temp_dir, "main.kn");
        let (promise, resolve) = Async.wait();

        FileUtil.cp(~recurse=true, [messy_fixture_dir], temp_dir);

        Format.run(
          ~catch=print_errs % Assert.fail,
          {...__compiler_config, root_dir: temp_dir},
          (),
        );

        Async.on_tick(() => {
          output_file |> Sys.file_exists |> Assert.true_;
          output_file
          |> Util.read_file_to_string
          |> Assert.string("const ABC = 123;\n");

          resolve();
        })
        |> ignore;

        promise;
      }
    ),
    "run() - ignore cyclic errors"
    >~:: (
      (_, ()) => {
        let temp_dir = Util.get_temp_dir();
        let main_file = Filename.concat(temp_dir, "main.kn");
        let utils_file = Filename.concat(temp_dir, "utils.kn");
        let (promise, resolve) = Async.wait();

        FileUtil.cp(~recurse=true, [messy_cylic_fixture_dir], temp_dir);

        Format.run(
          ~catch=print_errs % Assert.fail,
          {...__compiler_config, root_dir: temp_dir},
          (),
        );

        Async.on_tick(() => {
          main_file |> Sys.file_exists |> Assert.true_;
          main_file
          |> Util.read_file_to_string
          |> Assert.string("import Utils from \"@/utils\";\n");

          utils_file |> Sys.file_exists |> Assert.true_;
          utils_file
          |> Util.read_file_to_string
          |> Assert.string("import Main from \"@/main\";\n");

          resolve();
        })
        |> ignore;

        promise;
      }
    ),
    "run() - ignore missing files"
    >~:: (
      (_, ()) => {
        let temp_dir = Util.get_temp_dir();
        let main_file = Filename.concat(temp_dir, "main.kn");
        let (promise, resolve) = Async.wait();

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

        Async.on_tick(() => {
          main_file |> Sys.file_exists |> Assert.true_;
          main_file
          |> Util.read_file_to_string
          |> Assert.string("import Utils from \"@/utils\";\n");

          resolve();
        })
        |> ignore;

        promise;
      }
    ),
  ];
