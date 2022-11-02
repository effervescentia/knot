open Kore;

module Build = Executable.Build;

let __mock_library_file = fixture("mock_library.kd");
let __cwd = fixture("every_feature");

let suite =
  "Build | Every Feature"
  >::: [
    "javascript - es6"
    >: (
      () => {
        let out_dir = "build/es6";
        let argv = [|
          "knotc",
          "build",
          "--target",
          "javascript-es6",
          "--out-dir",
          out_dir,
        |];
        let (global, config) = process_build_cmd(__cwd, argv);

        Build.run(
          ~report=_ => AST.Error.throw_all,
          global,
          {...config, ambient: __mock_library_file},
        );

        Assert.directory(
          "es6_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
    "javascript - common"
    >: (
      () => {
        let out_dir = "build/common";
        let argv = [|
          "knotc",
          "build",
          "--target",
          "javascript-common",
          "--out-dir",
          out_dir,
        |];
        let (global, config) = process_build_cmd(__cwd, argv);

        Build.run(
          ~report=_ => AST.Error.throw_all,
          global,
          {...config, ambient: __mock_library_file},
        );

        Assert.directory(
          "common_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
    "knot"
    >: (
      () => {
        let out_dir = "build/knot";
        let argv = [|
          "knotc",
          "build",
          "--target",
          "knot",
          "--out-dir",
          out_dir,
        |];
        let (global, config) = process_build_cmd(__cwd, argv);

        Build.run(
          ~report=_ => AST.Error.throw_all,
          global,
          {...config, ambient: __mock_library_file},
        );

        Assert.directory(
          "knot_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
  ];
