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
    "pp_usage()"
    >: (
      () =>
        [
          (
            "  knotc <command> ...

COMMANDS

  build     compile files to target in output directory
  watch     run build and incrementally rebuild changed files
  format    update code style and spacing
  lint      analyze code style and report on anti-patterns
  lsp       run an LSP-compliant server for integration with IDEs
  bundle    generate executable from source code
  develop   run a development server to enable continuous development

OPTIONS

  --foo

    used to control the application of foo
",
            (None, None, [__opt]) |> ~@Args.pp_usage,
          ),
          (
            "  knotc foo [options]

COMMAND OPTIONS

  --foo

    used to control the application of foo

OPTIONS

  -b, --bar (options: fizz, buzz)
    [default: true]
    [from config: false]

    used to control the application of bar
",
            (
              Some(
                Cmd.{
                  name: "foo",
                  opts: [__opt],
                  resolve: (_, _) => RunCmd.Develop({port: 8080}),
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
          ),
        ]
        |> Assert.(test_many(string))
    ),
  ];
