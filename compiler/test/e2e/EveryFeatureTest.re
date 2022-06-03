open Kore;

module Compiler = Compile.Compiler;
module Build = Executable.Build;
module Processor = Executable.Processor;

let __stdlib = Resource.Asset.find("stdlib.kd");
let __cwd = _fixture("every_feature");

let _process_build_cmd = argv => {
  let (global, cmd) =
    Processor.run(~cwd=__cwd, ~argv, ~color=false, __stdlib);
  let config =
    cmd
    |> (
      fun
      | Build(cfg) => cfg
      | _ => Assert.fail("expected to resolve the build command")
    );

  (global, config);
};

let suite =
  "Build | Every Feature"
  >::: [
    "JavaScript ES6"
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
        let (global, config) = _process_build_cmd(argv);

        Build.run(~report=_ => throw_all, global, config);

        Assert.directory(
          "es6_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
    "JavaScript Common"
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
        let (global, config) = _process_build_cmd(argv);

        Build.run(~report=_ => throw_all, global, config);

        Assert.directory(
          "common_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
    "Knot"
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
        let (global, config) = _process_build_cmd(argv);

        Build.run(~report=_ => throw_all, global, config);

        Assert.directory(
          "knot_build_cache" |> Filename.concat(__cwd),
          out_dir |> Filename.concat(__cwd),
        );
      }
    ),
  ];
