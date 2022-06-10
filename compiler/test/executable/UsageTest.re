open Kore;

module Usage = Executable.Usage;
module Task = Executable.Task;

let __arg =
  Argument.create(
    "foo",
    Arg.Bool(ignore),
    "used to control the application of foo",
  );

let suite =
  "Executable.Usage"
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
    used to control the application of foo
",
          (None, None, [__arg]) |> ~@Usage.pp,
        )
    ),
    "pp_usage() - sub-command usage"
    >: (
      () =>
        Assert.string(
          "  knotc foo [options]

COMMAND OPTIONS

  --foo
    used to control the application of foo

OPTIONS

  -b, --bar
    [options: fizz, buzz]
    [default: true]
    [from config: false]
    used to control the application of bar
",
          (
            Some(
              Command.{
                name: "foo",
                arguments: [__arg],
                resolve: (_, _) => Task.DevServe({port: 8080}),
              },
            ),
            Some(Config.defaults),
            [
              Argument.create(
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
          |> ~@Usage.pp,
        )
    ),
  ];
