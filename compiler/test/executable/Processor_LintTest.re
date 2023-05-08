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
  "Executable.Processor | Lint"
  >::: [
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "lint"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Lint({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               fix: true,
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "lint", "--root-dir", "root"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Lint({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               fix: false,
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "lint", "--root-dir", "project"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Lint({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               fix: true,
             }),
           ))
    ),
    "run() - with --fix argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "lint", "--fix"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Lint({root_dir: path_alternative_fixture_dir, fix: true}),
           ))
    ),
    "run() - override fix from config with --fix argument"
    >: (
      () =>
        Processor.run(
          ~cwd=bool_flags_fixture_dir,
          ~argv=[|__binary, "lint", "--fix"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
             },
             Task.Lint({root_dir: bool_flags_fixture_dir, fix: true}),
           ))
    ),
  ];
