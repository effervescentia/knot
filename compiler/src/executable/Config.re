/**
 * Parse command line args into config.
 */
open Kore;

type t = {
  debug: bool,
  compile: Compiler.config_t,
};

let __commands = [
  ("build", " compile files to target in output directory"),
  ("watch", " run build and incrementally rebuild changed files"),
  ("format", " update code style and spacing"),
  ("lint", " analyze code style and report on anti-patterns"),
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

let from_args = (): (t, Command.t) => {
  let debug = ref(false);
  let config = ref("");
  let entry = ref("");

  let cmd = ref(None);

  let options = ref([]);

  let (compile_opts, resolve_compile) = Opt.Shared.compile();

  let rec usage = () => {
    let fmt_command =
      Print.fmt("knotc %s") % Print.bold % Print.fmt("  %s [options]");

    switch (cmd^) {
    | None =>
      Print.bold("knotc") |> Print.fmt("  %s <command> ...") |> print_endline;

      Print.bold("\nCOMMANDS\n") |> print_endline;

      __commands |> _print_cmds;
    | Some((name, command_opts, _)) =>
      name |> fmt_command |> print_endline;

      if (!List.is_empty(command_opts)) {
        Print.bold("\nCOMMAND OPTIONS\n") |> print_endline;

        command_opts |> _print_opts;
      };
    };

    Print.bold("\nOPTIONS\n") |> print_endline;

    global_opts() |> _print_opts;

    exit(2);
  }
  and global_opts = () =>
    compile_opts
    @ [
      Opt.create(
        ~default=Opt.Default.String(".knot.yml"),
        "config",
        Arg.Set_string(config),
        "set the location of the knot config file",
      ),
      Opt.create(
        ~default=Opt.Default.Bool(false),
        "debug",
        Arg.Set(debug),
        "enable a higher level of logging",
      ),
      Opt.create("help", Arg.Unit(usage), "display this list of options"),
    ];

  /* capture and replace the inbuilt -help arg */
  let hidden_opts = [("-help", Arg.Unit(usage), "")];

  let set_cmd = ((_, opts, _) as m) => {
    cmd := Some(m);
    options :=
      (opts @ global_opts() |> List.map(Opt.to_config) |> List.flatten)
      @ hidden_opts;
  };

  let build_cmd = Command.of_build(Build.mode());
  let watch_cmd = Command.of_watch(Watch.mode());
  let format_cmd = Command.of_format(Format.mode());
  let lint_cmd = Command.of_lint(Lint.mode());
  let lsp_cmd = Command.of_lsp(LSP.mode());
  let bundle_cmd = Command.of_bundle(Bundle.mode());
  let develop_cmd = Command.of_develop(Develop.mode());

  options :=
    (global_opts() |> List.map(Opt.to_config) |> List.flatten) @ hidden_opts;

  try(
    Arg.parse_dynamic(
      options,
      x =>
        switch (cmd^, x) {
        | (None, "build") => set_cmd(build_cmd)
        | (None, "watch") => set_cmd(watch_cmd)
        | (None, "format") => set_cmd(format_cmd)
        | (None, "lint") => set_cmd(lint_cmd)
        | (None, "lsp") => set_cmd(lsp_cmd)
        | (None, "bundle") => set_cmd(bundle_cmd)
        | (None, "develop") => set_cmd(develop_cmd)
        | _ =>
          if (entry^ == "") {
            entry := Filename.resolve(x);
          } else {
            Print.fmt("unexpected argument: %s", x) |> panic;
          }
        },
      "knotc [options] <command>",
    )
  ) {
  | _ => ()
  };

  let (_, _, resolve) =
    cmd^
    |!: (
      () => {
        __commands |> _print_cmds;
        print_newline();

        panic("must provide a command");
      }
    );

  ({debug: debug^, compile: resolve_compile()}, resolve());
};
