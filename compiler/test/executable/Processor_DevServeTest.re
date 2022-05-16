open Executable.Kore;
open Kore;

module Processor = Executable.Processor;
module Task = Executable.Task;

let __binary = "knotc.exe";

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
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_override",
               working_dir: path_override_fixture_dir,
               color: false,
               debug: false,
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
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "no_target",
               working_dir: no_target_fixture_dir,
               color: false,
               debug: false,
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
          (),
        )
        |> Assert.task_with_config((
             Config.{
               name: "path_override",
               working_dir: path_override_fixture_dir,
               color: false,
               debug: false,
             },
             Task.DevServe({port: 1337}),
           ))
    ),
  ];
