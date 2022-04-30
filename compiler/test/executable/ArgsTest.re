open Kore;

module Args = Executable.Args;
module Cmd = Executable.Cmd;
module Opt = Executable.Opt;
module RunCmd = Executable.RunCmd;

let __opt =
  Opt.create(
    "foo",
    Arg.Bool(ignore),
    "used to control the application of foo",
  );

let suite =
  "Executable.Args"
  >::: [
    "pp_usage() - root command usage"
    >: (
      () =>
        Assert.string(
          "  knotc <command> ...

COMMANDS

  build         compile files to target in output directory
  watch         run build and incrementally rebuild changed files
  format        update code style and spacing
  lint          analyze code style and report on anti-patterns
  bundle        generate executable from source code
  dev_serve     run a development server to enable continuous development
  lang_serve    run an LSP-compliant server for integration with IDEs
  build_serve   run a JSONRPC server that can perform incremental compilation

OPTIONS

  --foo
    \n    used to control the application of foo
",
          (None, None, [__opt]) |> ~@Args.pp_usage,
        )
    ),
    "pp_usage() - sub-command usage"
    >: (
      () =>
        Assert.string(
          "  knotc foo [options]

COMMAND OPTIONS

  --foo
    \n    used to control the application of foo

OPTIONS

  -b, --bar
    [options: fizz, buzz]
    [default: true]
    [from config: false]
    \n    used to control the application of bar
",
          (
            Some(
              Cmd.{
                name: "foo",
                opts: [__opt],
                resolve: (_, _) => RunCmd.DevServe({port: 8080}),
              },
            ),
            Some(Config.defaults(false)),
            [
              Opt.create(
                ~alias="b",
                ~default=Bool(true),
                ~from_config=_ => Some(Bool(false)),
                ~options=["fizz", "buzz"],
                "bar",
                Arg.Bool(ignore),
                "used to control the application of bar",
              ),
            ],
          )
          |> ~@Args.pp_usage,
        )
    ),
  ];
