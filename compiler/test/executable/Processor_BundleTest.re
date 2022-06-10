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
  "Executable.Processor | Bundle"
  >::: [
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "bundle"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Bundle({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "out",
             }),
           ))
    ),
    "run() - with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "bundle", "--root-dir", "root"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Bundle({
               root_dir:
                 Filename.concat(path_alternative_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "build",
             }),
           ))
    ),
    "run() - override root_dir from config with --root-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "bundle", "--root-dir", "project"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Bundle({
               root_dir:
                 Filename.concat(path_override_fixture_dir, "project"),
               source_dir: "lib",
               out_dir: "out",
             }),
           ))
    ),
    "run() - with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "bundle", "--source-dir", "lib"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Bundle({
               root_dir: path_alternative_fixture_dir,
               source_dir: "lib",
               out_dir: "build",
             }),
           ))
    ),
    "run() - override source_dir from config with --source-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "bundle", "--source-dir", "src"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Bundle({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "src",
               out_dir: "out",
             }),
           ))
    ),
    "run() - with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_alternative_fixture_dir,
          ~argv=[|__binary, "bundle", "--out-dir", "output"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_alternative",
               working_dir: path_alternative_fixture_dir,
             },
             Task.Bundle({
               root_dir: path_alternative_fixture_dir,
               source_dir: "src",
               out_dir: "output",
             }),
           ))
    ),
    "run() - override out_dir from config with --out-dir argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "bundle", "--out-dir", "build"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.Bundle({
               root_dir: Filename.concat(path_override_fixture_dir, "root"),
               source_dir: "lib",
               out_dir: "build",
             }),
           ))
    ),
  ];
