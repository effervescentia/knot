open Executable.Kore;
open Kore;

module Processor = Executable.Processor;
module Task = Executable.Task;

let __binary = "knotc.exe";

let suite =
  "Executable.Processor | Build"
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
            ~argv=[|__binary, "build"|],
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
          ~argv=[|__binary, "build"|],
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
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "build", "--target", "knot"|],
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
             Task.Build({
               root_dir: no_target_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.Knot,
               fail_fast: false,
               log_imports: false,
             }),
           ))
    ),
    "run() - override target from config with --target argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build", "--target", "javascript-es6"|],
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
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(ES6),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--root-dir", "root"|],
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
             Task.Build({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build", "--root-dir", "project"|],
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
             Task.Build({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--source-dir", "lib"|],
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
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
             }),
           ))
    ),
    "run() - override source_dir from config with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build", "--source-dir", "src"|],
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
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--out-dir", "output"|],
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
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "output",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
             }),
           ))
    ),
    "run() - override out_dir from config with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build", "--out-dir", "build"|],
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
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --entry argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--entry", "index.kn"|],
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
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
             }),
           ))
    ),
    "run() - override entry from config with --entry argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "build", "--entry", "main.kn"|],
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
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
             }),
           ))
    ),
    "run() - with --fail-fast argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--fail-fast"|],
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
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: false,
             }),
           ))
    ),
    "run() - override fail_fast from config with --fail-fast argument"
    >: (
      () =>
        Processor.run(
          ~cwd=bool_flags_fixture_dir,
          ~argv=[|__binary, "build", "--fail-fast"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
               color: false,
               debug: false,
             },
             Task.Build({
               root_dir: bool_flags_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: false,
             }),
           ))
    ),
    "run() - with --log-imports argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "build", "--log-imports"|],
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
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: true,
             }),
           ))
    ),
    "run() - override log_imports from config with --log-imports argument"
    >: (
      () =>
        Processor.run(
          ~cwd=bool_flags_fixture_dir,
          ~argv=[|__binary, "build", "--log-imports"|],
          ~color=false,
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
               color: false,
               debug: false,
             },
             Task.Build({
               root_dir: bool_flags_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: true,
             }),
           ))
    ),
  ];
