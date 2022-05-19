open Executable.Kore;
open Kore;

module Processor = Executable.Processor;
module Task = Executable.Task;

let __binary = "knotc.exe";

let suite =
  "Executable.Processor | BuildServe"
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
            ~argv=[|__binary, "build_serve"|],
            ~color=false,
            (),
          )
        )
    ),
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build_serve"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_override",
               working_dir: path_override_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               target: Target.JavaScript(Common),
               log_imports: true,
             }),
           ))
    ),
    "run() - with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "build_serve", "--target", "knot"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "no_target",
               working_dir: no_target_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir: no_target_fixture_dir,
               source_dir: "src",
               target: Target.Knot,
               log_imports: false,
             }),
           ))
    ),
    "run() - override target from config with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=js_target_fixture_dir,
          ~argv=[|__binary, "build_serve", "--target", "knot"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "js_target",
               working_dir: js_target_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir: js_target_fixture_dir,
               source_dir: "src",
               target: Target.Knot,
               log_imports: false,
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build_serve", "--root-dir", "root"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               source_dir: "src",
               target: Target.JavaScript(Common),
               log_imports: false,
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build_serve", "--root-dir", "project"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_override",
               working_dir: path_override_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               source_dir: "lib",
               target: Target.JavaScript(Common),
               log_imports: true,
             }),
           ))
    ),
    "run() - with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build_serve", "--source-dir", "lib"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
               target: Target.JavaScript(Common),
               log_imports: false,
             }),
           ))
    ),
    "run() - override source_dir from config with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build_serve", "--source-dir", "src"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_override",
               working_dir: path_override_fixture_dir,
               color: false,
               debug: false,
             },
             Task.BuildServe({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
               target: Target.JavaScript(Common),
               log_imports: true,
             }),
           ))
    ),
  ];
