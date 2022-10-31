open Kore;

module Processor = Executable.Processor;
module Task = Executable.Task;

let __binary = "knotc.exe";
let __stdlib = "stdlib.kd";

let suite =
  "Executable.Processor"
  >::: [
    "run() - with options from config"
    >: (
      () =>
        Fun.protect(
          ~finally=() => Fmt.color := false,
          () =>
            Processor.run(
              ~cwd=global_config_fixture_dir,
              ~argv=[|__binary, "lang_serve"|],
              ~color=false,
              __stdlib,
            )
            |> Assert.task_with_config((
                 Config.{
                   name: "foobar",
                   working_dir: global_config_fixture_dir,
                   color: true,
                   debug: true,
                   stdlib: __stdlib,
                 },
                 Task.LangServe(),
               )),
        )
    ),
    "run() - with --cwd relative argument"
    >: (
      () =>
        Processor.run(
          ~cwd=fixture_dir,
          ~argv=[|__binary, "lang_serve", "--cwd", "nested/deeper"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "nested",
               working_dir: Filename.concat(nested_fixture_dir, "deeper"),
               color: false,
               debug: false,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ))
    ),
    "run() - with --cwd absolute argument"
    >: (
      () => {
        let working_dir =
          Filename.concat(Filename.resolve(nested_fixture_dir), "deeper");

        Processor.run(
          ~argv=[|__binary, "lang_serve", "--cwd", working_dir|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "nested",
               working_dir,
               color: false,
               debug: false,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ));
      }
    ),
    "run() - with --config relative argument"
    >: (
      () => {
        let working_dir = Filename.concat(nested_fixture_dir, "deeper");

        Processor.run(
          ~cwd=fixture_dir,
          ~argv=[|
            __binary,
            "lang_serve",
            "--config",
            "nested/deeper/.knot.yml",
          |],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "nested",
               working_dir,
               color: false,
               debug: false,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ));
      }
    ),
    "run() - with --config absolute argument"
    >: (
      () => {
        let working_dir =
          Filename.concat(Filename.resolve(nested_fixture_dir), "deeper");
        let config_file = Filename.concat(working_dir, ".knot.yml");

        Processor.run(
          ~argv=[|__binary, "lang_serve", "--config", config_file|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "nested",
               working_dir,
               color: false,
               debug: false,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ));
      }
    ),
    "run() - with relative --cwd, --config and --root-dir arguments"
    >: (
      () =>
        Processor.run(
          ~cwd=fixture_dir,
          ~argv=[|
            __binary,
            "lint",
            "--cwd",
            "nested",
            "--config",
            "deeper/.knot.yml",
            "--root-dir",
            "root",
          |],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "nested",
               working_dir: Filename.concat(nested_fixture_dir, "deeper"),
               color: false,
               debug: false,
               stdlib: __stdlib,
             },
             Task.Lint({
               root_dir: Filename.concat(nested_fixture_dir, "deeper/root"),
               fix: false,
             }),
           ))
    ),
    "run() - override default color"
    >: (
      () =>
        Fun.protect(
          ~finally=() => Fmt.color := false,
          () =>
            Processor.run(
              ~cwd=no_target_fixture_dir,
              ~argv=[|__binary, "lang_serve"|],
              ~color=true,
              __stdlib,
            )
            |> Assert.task_with_config((
                 Config.{
                   name: "no_target",
                   working_dir: no_target_fixture_dir,
                   color: true,
                   debug: false,
                   stdlib: __stdlib,
                 },
                 Task.LangServe(),
               )),
        )
    ),
    "run() - with --color argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "lang_serve", "--color", "true"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "no_target",
               working_dir: no_target_fixture_dir,
               color: true,
               debug: false,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ))
    ),
    "run() - override color from config with --color argument"
    >: (
      () =>
        Processor.run(
          ~cwd=global_config_fixture_dir,
          ~argv=[|__binary, "lang_serve", "--color", "false"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "foobar",
               working_dir: global_config_fixture_dir,
               color: false,
               debug: true,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ))
    ),
    "run() - with --debug argument"
    >: (
      () =>
        Processor.run(
          ~cwd=no_target_fixture_dir,
          ~argv=[|__binary, "lang_serve", "--debug"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "no_target",
               working_dir: no_target_fixture_dir,
               color: false,
               debug: true,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ))
    ),
    "run() - override debug from config with --debug argument"
    >: (
      () =>
        Processor.run(
          ~cwd=bool_flags_fixture_dir,
          ~argv=[|__binary, "lang_serve", "--debug"|],
          ~color=false,
          __stdlib,
        )
        |> Assert.task_with_config((
             Config.{
               name: "bool_flags",
               working_dir: bool_flags_fixture_dir,
               color: false,
               debug: true,
               stdlib: __stdlib,
             },
             Task.LangServe(),
           ))
    ),
  ];
