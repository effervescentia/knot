open Kore;

module Format = Executable.Format;

let __compiler_config = Config.{name: "foo", debug: false, color: false};
let __source_dir = ".";

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
          ~report=_ => ~@pp_dump_err_list % Assert.fail,
          __compiler_config,
          {root_dir: temp_dir, source_dir: __source_dir},
        );

        Async.on_tick(() => {
          Assert.true_(Sys.file_exists(output_file));
          Assert.string(
            "const ABC = 123;\n",
            Util.read_file_to_string(output_file),
          );

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

        FileUtil.cp(~recurse=true, [messy_cyclic_fixture_dir], temp_dir);

        Format.run(
          ~report=_ => ~@pp_dump_err_list % Assert.fail,
          __compiler_config,
          {root_dir: temp_dir, source_dir: __source_dir},
        );

        Async.on_tick(() => {
          Assert.true_(Sys.file_exists(main_file));
          Assert.string(
            "import Utils from \"@/utils\";\n",
            Util.read_file_to_string(main_file),
          );

          Assert.true_(Sys.file_exists(utils_file));
          Assert.string(
            "import Main from \"@/main\";\n",
            Util.read_file_to_string(utils_file),
          );

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
          ~report=_ => ~@pp_dump_err_list % Assert.fail,
          __compiler_config,
          {root_dir: temp_dir, source_dir: __source_dir},
        );

        Async.on_tick(() => {
          Assert.true_(Sys.file_exists(main_file));
          Assert.string(
            "import Utils from \"@/utils\";\n",
            Util.read_file_to_string(main_file),
          );

          resolve();
        })
        |> ignore;

        promise;
      }
    ),
  ];
