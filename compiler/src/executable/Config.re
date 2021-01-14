open Kore;

module ANSI = ANSITerminal;

type t = {
  mode: Mode.t,
  entry: string,
  root_dir: string,
  globs: list(string),
  debug: bool,
};

let _bold = ANSI.sprintf([ANSI.Bold], "%s");

let _bold_items = List.map(((name, desc)) => (_bold(name), desc));

let _pad_items = xs => {
  let offset =
    xs
    |> List.map(((x, _)) => String.length(x))
    |> List.fold_left((acc, len) => len > acc ? len : acc, 0)
    |> (+)(2);

  xs |> List.map(((cmd, desc)) => (Print.fmt("  %-*s", offset, cmd), desc));
};

let _print_items =
  _bold_items
  % _pad_items
  % Print.many(~separator="\n", ((cmd, desc)) => cmd ++ desc)
  % print_endline;

let command_args = [
  ("build", " compile files to target in output directory"),
  ("watch", " run build and incrementally rebuild changed files"),
  ("format", " update code style and spacing"),
  ("lint", " analyze code style and report for anti-patterns"),
  ("lsp", " run an LSP-compliant server for integration with IDEs"),
  ("bundle", " generate executable from source code"),
  ("develop", " run a development server to enable continuous development"),
];
/* |> List.map(((x, y)) => (x, Arg.Unit(Functional.identity), y)); */

let from_args = (): t => {
  let debug = ref(false);
  let fix = ref(false);
  let config = ref("");
  let entry = ref("");
  let root_dir = ref("");
  let globs = ref([]);
  let port = ref(1337);

  let mode = ref(None);

  let args = ref([]);

  let rec usage = () => {
    let fmt_command =
      Print.fmt("knotc %s") % _bold % Print.fmt("  %s [options]");

    switch (mode^) {
    | None =>
      _bold("knotc") |> Print.fmt("  %s <command> ...") |> print_endline;

      _bold("\nCOMMANDS\n") |> print_endline;

      command_args |> _print_items;
    | Some(Mode.Build) => fmt_command("build") |> print_endline
    | Some(Mode.Watch) => fmt_command("watch") |> print_endline
    | Some(Mode.Format) => fmt_command("format") |> print_endline
    | Some(Mode.Lint) => fmt_command("lint") |> print_endline
    | Some(Mode.LSP) => fmt_command("lsp") |> print_endline
    | Some(Mode.Bundle) => fmt_command("bundle") |> print_endline
    | Some(Mode.Develop) => fmt_command("develop") |> print_endline
    };

    let options =
      args^
      |> List.excl_all(hidden_args @ global_args)
      |> List.map(((name, _, desc)) => (name, desc));

    if (!List.is_empty(options)) {
      _bold("\nCOMMAND OPTIONS\n") |> print_endline;

      options |> _print_items;
    };

    _bold("\nOPTIONS\n") |> print_endline;

    global_args
    |> List.excl_all(hidden_args)
    |> List.map(((name, _, desc)) => (name, desc))
    |> _print_items;

    exit(2);
  }
  /* capture and replace the inbuilt -help arg */
  and hidden_args = [("-help", Arg.Unit(usage), "")]
  and global_args = [
    (
      "--config",
      Arg.Set_string(config),
      " set the location of the knot config file",
    ),
    ("--debug", Arg.Set(debug), " enable a higher level of logging"),
    ("--help", Arg.Unit(usage), " display this list of options"),
    ...hidden_args,
  ];

  let file_args = [
    (
      "--root-dir",
      Arg.Set_string(root_dir),
      " the root directory to reference modules from",
    ),
  ];

  let build_args =
    [
      ("--target", Arg.Set_string(root_dir), " the target to compile to"),
      (
        "--out-dir",
        Arg.Set_string(root_dir),
        " the directory to write compiled files to",
      ),
    ]
    @ file_args
    @ global_args;

  let watch_args = [] @ file_args @ global_args;

  let format_args = [] @ file_args @ global_args;

  let lint_args =
    [("--fix", Arg.Set(fix), " automatically apply fixes")]
    @ file_args
    @ global_args;

  let lsp_args =
    [("--port", Arg.Set_int(port), " the port the server runs on")]
    @ global_args;

  let bundle_args = [] @ global_args;

  let develop_args =
    [("--port", Arg.Set_int(port), " the port the server runs on")]
    @ global_args;

  args := global_args;

  Arg.parse_dynamic(
    args,
    x =>
      switch (mode^, x) {
      | (None, "build") =>
        mode := Some(Mode.Build);
        args := build_args;
      | (None, "watch") =>
        mode := Some(Mode.Watch);
        args := watch_args;
      | (None, "format") =>
        mode := Some(Mode.Format);
        args := format_args;
      | (None, "lint") =>
        mode := Some(Mode.Lint);
        args := lint_args;
      | (None, "lsp") =>
        mode := Some(Mode.LSP);
        args := lsp_args;
      | (None, "bundle") =>
        mode := Some(Mode.Bundle);
        args := bundle_args;
      | (None, "develop") =>
        mode := Some(Mode.Develop);
        args := develop_args;
      | _ =>
        if (entry^ == "") {
          entry := Filename.normalize(x);
        } else {
          raise(Arg.Bad(Print.fmt("unexpected argument: %s", x)));
        }
      },
    "knotc [options] <command>",
  );

  let mode = mode^ |?: Mode.Build;
  let entry = Filename.basename(entry^);
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
