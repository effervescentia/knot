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
  "Executable.Processor | Format"
  >::: [
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "format"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Format({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "format", "--root-dir", "root"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Format({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               source_dir: "src",
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "format", "--root-dir", "project"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Format({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               source_dir: "lib",
             }),
           ))
    ),
    "run() - with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "format", "--source-dir", "lib"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Format({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
             }),
           ))
    ),
    "run() - override source_dir from config with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "format", "--source-dir", "src"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Format({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
             }),
           ))
    ),
  ];
