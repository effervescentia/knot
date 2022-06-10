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
  "Executable.Processor | DevServe"
  >::: [
    "run() - with options from config"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "dev_serve"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.DevServe({port: 1338}),
           ))
    ),
    "run() - with --port argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "dev_serve", "--port", "1338"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "no_target",
               working_dir: no_target_fixture_dir,
             },
             Task.DevServe({port: 1338}),
           ))
    ),
    "run() - override port from config with --port argument"
    >: (
      () =>
        Processor.run(
          ~cwd=path_override_fixture_dir,
          ~argv=[|__binary, "dev_serve", "--port", "1337"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               ...__config,
               name: "path_override",
               working_dir: path_override_fixture_dir,
             },
             Task.DevServe({port: 1337}),
           ))
    ),
  ];
