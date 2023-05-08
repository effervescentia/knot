open Executable.Kore;
open Kore;

module Namespace = Reference.Namespace;
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
            __stdlib,
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "no_target",
               working_dir: no_target_fixture_dir,
             },
             Task.Build({
               root_dir: no_target_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.Knot,
               fail_fast: false,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/core.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(ES6),
               fail_fast: true,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
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
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
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
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "out",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "output",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("index"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Build({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
             },
             Task.Build({
               root_dir: bool_flags_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: true,
               log_imports: false,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Build({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
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
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
             },
             Task.Build({
               root_dir: bool_flags_fixture_dir,
               source_dir: "src",
               out_dir: "build",
               entry: Namespace.Internal("main"),
               target: Target.JavaScript(Common),
               fail_fast: false,
               log_imports: true,
               ambient: "../share/knot/assets/ambient/browser.kd",
             }),
           ))
    ),
  ];
