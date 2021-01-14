open Kore;

type t = {
  mode: Mode.t,
  entry: string,
  root_dir: string,
  globs: list(string),
  debug: bool,
};

let __commands = [
  ("build", " compile files to target in output directory"),
  ("watch", " run build and incrementally rebuild changed files"),
  ("format", " update code style and spacing"),
  ("lint", " analyze code style and report for anti-patterns"),
  ("lsp", " run an LSP-compliant server for integration with IDEs"),
  ("bundle", " generate executable from source code"),
  ("develop", " run a development server to enable continuous development"),
];

let __targets = ["javascript"];

let _bold_items = List.map(((name, desc)) => (Print.bold(name), desc));

let _pad_items = xs => {
  let offset =
    xs
    |> List.map(((x, _)) => String.length(x))
    |> List.fold_left((acc, len) => len > acc ? len : acc, 0)
    |> (+)(2);

  xs |> List.map(((cmd, desc)) => (Print.fmt("  %-*s", offset, cmd), desc));
};

let _print_cmds =
  _bold_items
  % _pad_items
  % Print.many(~separator="\n", ((cmd, desc)) => cmd ++ desc)
  % print_endline;

let _print_opts = xs =>
  Print.many(~separator="\n\n", Opt.to_string, xs) |> print_endline;

let from_args = (): t => {
  let debug = ref(false);
  let fix = ref(false);
  let config = ref("");
  let entry = ref("");
  let root_dir = ref("");
  let out_dir = ref("");
  let target = ref(None);
  let globs = ref([]);
  let port = ref(1337);

  let mode = ref(None);

  let options = ref([]);
  let command_opts = ref([]);

  let rec usage = () => {
    let fmt_command =
      Print.fmt("knotc %s") % Print.bold % Print.fmt("  %s [options]");

    switch (mode^) {
    | None =>
      Print.bold("knotc") |> Print.fmt("  %s <command> ...") |> print_endline;

      Print.bold("\nCOMMANDS\n") |> print_endline;

      __commands |> _print_cmds;
    | Some(mode) => Mode.to_string(mode) |> fmt_command |> print_endline
    };

    if (!List.is_empty(command_opts^)) {
      Print.bold("\nCOMMAND OPTIONS\n") |> print_endline;

      command_opts^ |> _print_opts;
    };

    Print.bold("\nOPTIONS\n") |> print_endline;

    global_opts() |> _print_opts;

    exit(2);
  }
  and global_opts = () => [
    Opt.create(
      ~default=Opt.Default.String(".knot.yml"),
      "config",
      Arg.Set_string(config),
      "set the location of the knot config file",
    ),
    Opt.create("debug", Arg.Set(debug), "enable a higher level of logging"),
    Opt.create("help", Arg.Unit(usage), "display this list of options"),
  ];

  /* capture and replace the inbuilt -help arg */
  let hidden_opts = [("-help", Arg.Unit(usage), "")];

  let set_mode = (m, opts) => {
    mode := Some(m);
    command_opts := opts;
    options :=
      (opts @ global_opts() |> List.map(Opt.to_config) |> List.flatten)
      @ hidden_opts;
  };

  let file_opts = [
    Opt.create(
      "root-dir",
      Arg.Set_string(root_dir),
      "the root directory to reference modules from",
    ),
  ];

  let server_opts = [
    Opt.create(
      ~alias="p",
      ~default=Opt.Default.Int(port^),
      "port",
      Arg.Set_int(port),
      "the port the server runs on",
    ),
  ];

  let compile_opts = [
    Opt.create(
      ~alias="t",
      ~options=__targets,
      "target",
      Arg.Symbol(
        __targets,
        fun
        | "javascript" => target := Some(Target.JavaScript(Target.ES6))
        | x => Print.fmt("unknown target: '%s'", x) |> panic,
      ),
      "the target to compile to",
    ),
  ];

  let build_opts =
    compile_opts
    @ [
      Opt.create(
        ~alias="o",
        "out-dir",
        Arg.Set_string(out_dir),
        "the directory to write compiled files to",
      ),
    ]
    @ file_opts;

  let watch_opts = compile_opts @ [] @ file_opts;

  let format_opts = [] @ file_opts;

  let lint_opts =
    [
      Opt.create(
        ~alias="f",
        ~default=Opt.Default.Bool(fix^),
        "fix",
        Arg.Set(fix),
        "automatically apply fixes",
      ),
    ]
    @ file_opts;

  let lsp_opts = [] @ server_opts;

  let bundle_opts = [];

  let develop_opts = [] @ server_opts;

  options :=
    (global_opts() |> List.map(Opt.to_config) |> List.flatten) @ hidden_opts;

  try(Arg.parse_dynamic(
    options,
    x =>
      switch (mode^, x) {
      | (None, "build") => set_mode(Mode.Build, build_opts)
      | (None, "watch") => set_mode(Mode.Watch, watch_opts)
      | (None, "format") => set_mode(Mode.Format, format_opts)
      | (None, "lint") => set_mode(Mode.Lint, lint_opts)
      | (None, "lsp") => set_mode(Mode.LSP, lsp_opts)
      | (None, "bundle") => set_mode(Mode.Build, bundle_opts)
      | (None, "develop") => set_mode(Mode.Develop, develop_opts)
      | _ =>
        if (entry^ == "") {
          entry := Filename.normalize(x);
        } else {
          Print.fmt("unexpected argument: %s", x) |> panic;
        }
      },
    "knotc [options] <command>",
  )) {
   | _ => ()
  };

  let mode =
    mode^
    |!: (
      () => {
        __commands |> _print_cmds;
        print_newline();

        panic("must provide a command");
      }
    );
  let entry = entry^ == "" ? panic("must provide an entry") : Filename.basename(entry^);
  let globs = globs^;
  let debug = debug^;

  {
    mode,
    entry,
    root_dir: root_dir^ == "" ? Filename.dirname(entry) : root_dir^,
    globs,

    debug,
  };
};
