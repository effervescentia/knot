/**
 Parse command line args into config.
 */
open Kore;

let __commands = [
  (build_key, " compile files to target in output directory"),
  (watch_key, " run build and incrementally rebuild changed files"),
  (format_key, " update code style and spacing"),
  (lint_key, " analyze code style and report on anti-patterns"),
  (lsp_key, " run an LSP-compliant server for integration with IDEs"),
  (bundle_key, " generate executable from source code"),
  (develop_key, " run a development server to enable continuous development"),
];

let __config_file = ".knot.yml";

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

let _print_opts = (cfg, xs) =>
  Print.many(~separator="\n\n", Opt.to_string(cfg), xs) |> print_endline;
let from_file = (file: string): static_t => {
  let root_dir = ref(defaults.root_dir);
  let source_dir = ref(defaults.source_dir);
  let out_dir = ref(defaults.out_dir);
  let target = ref(defaults.target);
  let entry = ref(defaults.entry);
  let color = ref(defaults.color);
  let fix = ref(defaults.fix);
  let debug = ref(defaults.debug);
  let port = ref(defaults.port);

  switch (file |> File.IO.read_to_string |> Yaml.of_string) {
  | Ok(`O(entries)) =>
    entries
    |> List.iter(
         fun
         | (name, `String(value)) when name == root_dir_key =>
           root_dir := value |> Filename.resolve
         | (name, `String(value)) when name == source_dir_key =>
           source_dir := value
         | (name, `String(value)) when name == out_dir_key =>
           out_dir := value
         | (name, `String(value)) when name == target_key =>
           target := Some(target_of_string(value))
         | (name, `String(value)) when name == entry_key => entry := value
         | (name, `Bool(value)) when name == fix_key => fix := value
         | (name, `Bool(value)) when name == color_key => color := value
         | (name, `Bool(value)) when name == debug_key => debug := value
         | (name, `Float(value)) when name == port_key =>
           port := value |> int_of_float
         | (name, _) =>
           name |> Print.fmt("invalid entry found: %s") |> panic,
       )
  | Ok(_) =>
    Print.fmt(
      "expected an object with some of the following keys: root_dir, source_dir, entry",
    )
    |> panic
  | _ => file |> Print.fmt("failed to parse configuration file: %s") |> panic
  };

  {
    root_dir: root_dir^,
    source_dir: source_dir^,
    out_dir: out_dir^,
    target: target^,
    entry: entry^,
    color: color^,
    fix: fix^,
    debug: debug^,
    port: port^,
  };
};

let from_args = (): (global_t, RunCmd.t) => {
  let auto_config_file = File.Util.find_up(__config_file, defaults.root_dir);
  let config_file = ref("");
  let static = ref(auto_config_file |?> from_file);

  let cmd = ref(None);

  let options = ref([]);

  let (debug_opt, get_debug) = ConfigOpt.debug();
  let (root_dir_opt, get_root_dir) = ConfigOpt.root_dir();
  let (source_dir_opt, get_source_dir) = ConfigOpt.source_dir();

  let find_static_config = root_dir =>
    if (config_file^ == "") {
      switch (File.Util.find_up(__config_file, root_dir)) {
      | Some(path) =>
        static := Some(path |> from_file);
        config_file := path;
      | None => ()
      };
    };

  let rec usage = () => {
    find_static_config(get_root_dir(static^));

    let fmt_command = ({name}: Cmd.t('a)) =>
      Print.fmt(
        "  %s [options]",
        name |> Print.fmt("knotc %s") |> Print.bold,
      );

    switch (cmd^) {
    | None =>
      Print.bold("knotc") |> Print.fmt("  %s <command> ...") |> print_endline;

      Print.bold("\nCOMMANDS\n") |> print_endline;

      __commands |> _print_cmds;
    | Some(Cmd.{name, opts: command_opts} as cmd) =>
      cmd |> fmt_command |> print_endline;

      if (!List.is_empty(command_opts)) {
        Print.bold("\nCOMMAND OPTIONS\n") |> print_endline;

        command_opts |> _print_opts(static^);
      };
    };

    Print.bold("\nOPTIONS\n") |> print_endline;

    global_opts() |> _print_opts(static^);

    exit(2);
  }
  and global_opts = () => [
    root_dir_opt,
    source_dir_opt,
    Opt.create(
      ~alias="c",
      ~default=String(auto_config_file |?: __config_file),
      "config",
      String(
        Filename.resolve
        % (
          x => {
            if (!Sys.file_exists(x)) {
              x
              |> Print.fmt("unable to find the specified config file: %s")
              |> panic;
            };

            static := Some(from_file(x));
            config_file := x;
          }
        ),
      ),
      "set the location of the knot config file",
    ),
    debug_opt,
    Opt.create("help", Unit(usage), "display this list of options"),
  ];

  /* capture and replace the inbuilt -help arg */
  let hidden_opts = [("-help", Arg.Unit(usage), "")];

  let set_cmd = ({opts} as m: Cmd.t('a)) => {
    cmd := Some(m);
    options :=
      (opts @ global_opts() |> List.map(Opt.to_args) |> List.flatten)
      @ hidden_opts;
  };

  options :=
    (global_opts() |> List.map(Opt.to_args) |> List.flatten) @ hidden_opts;

  try(
    Arg.parse_dynamic(
      options,
      x =>
        switch (cmd^, x) {
        | (None, key) when key == build_key => set_cmd(RunCmd.build)
        | (None, key) when key == watch_key => set_cmd(RunCmd.watch)
        | (None, key) when key == format_key => set_cmd(RunCmd.format)
        | (None, key) when key == lint_key => set_cmd(RunCmd.lint)
        | (None, key) when key == lsp_key => set_cmd(RunCmd.lsp)
        | (None, key) when key == bundle_key => set_cmd(RunCmd.bundle)
        | (None, key) when key == develop_key => set_cmd(RunCmd.develop)
        | _ => Print.fmt("unexpected argument: %s", x) |> panic
        },
      "knotc [options] <command>",
    )
  ) {
  | _ => ()
  };

  let Cmd.{resolve} =
    cmd^
    |!: (
      () => {
        __commands |> _print_cmds;
        print_newline();

        panic("must provide a command");
      }
    );

  let root_dir = get_root_dir(static^);
  find_static_config(root_dir);

  let global_cfg = {
    debug: get_debug(static^),
    root_dir,
    source_dir: get_source_dir(static^, root_dir),
    name: root_dir |> Filename.basename,
  };

  (global_cfg, resolve(static^, global_cfg));
};
