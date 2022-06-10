open Executable.Kore;
open Kore;

module Processor = Executable.Processor;
module Task = Executable.Task;

let __binary = "knotc.exe";
let __stdlib = "stdlib.kd";
let __config =
  Config.{
    name: "",
    working_dir: "",
    color: false,
    debug: false,
    stdlib: __stdlib,
  };

let suite =
  "Executable.Processor | Watch"
  >::: [
    "run() - with no target"
    >: (
      () =>
        Assert.throws(
          FatalError(
            InvalidArgument(
              "target",
              "must provide a target for compilation",
            ),
          ),
          "expected error if no target provided",
          () =>
          Processor.run(
            ~cwd=no_target_fixture_dir,
            ~argv=[|__binary, "watch"|],
            ~color=false,
            __stdlib,
          )
        )
    ),
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "watch", "--target", "knot"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "no_target",
               working_dir: no_target_fixture_dir,
             },
             Task.Watch({
               root_dir: no_target_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.Knot,
             }),
           ))
    ),
    "run() - override target from config with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch", "--target", "javascript-es6"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(ES6),
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "watch", "--root-dir", "root"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Watch({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch", "--root-dir", "project"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "watch", "--source-dir", "lib"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Watch({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - override source_dir from config with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch", "--source-dir", "src"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "watch", "--out-dir", "output"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Watch({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "output",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - override out_dir from config with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch", "--out-dir", "build"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - with --entry argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "watch", "--entry", "index.kn"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Watch({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
    "run() - override entry from config with --entry argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "watch", "--entry", "main.kn"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Watch({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
             }),
           ))
    ),
  ];
