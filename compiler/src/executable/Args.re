/**
 Parse command line args into config.
 */
open Kore;

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

let to_config = (): (global_t, RunCmd.t) => {
  let auto_config_file = ConfigFile.find(default_config.root_dir);
  let config_file = ref(None);
  let static =
    ref(
      auto_config_file
      |?> (
        file => {
          Log.debug("automatically found config file: %s", file);
          ConfigFile.read(file);
        }
      ),
    );

  let cmd = ref(None);

  let options = ref([]);

  let (debug_opt, get_debug) = ConfigOpt.debug();
  let (color_opt, get_color) = ConfigOpt.color();
  let (root_dir_opt, get_root_dir) = ConfigOpt.root_dir();

  let find_static_config = root_dir =>
    if (config_file^ == None) {
      switch (File.Util.find_up(ConfigFile.name, root_dir)) {
      | Some(path) =>
        static := Some(ConfigFile.read(path));
        config_file := Some(path);
      | None => ()
      };
    };

  let rec usage = () => {
    find_static_config(get_root_dir(static^));

    if (!is_ci) {
      Print.color := true;
    };

    let fmt_command = ({name}: Cmd.t('a)) =>
      Print.fmt(
        "  %s [options]",
        name |> Print.fmt("knotc %s") |> Print.bold,
      );

    switch (cmd^) {
    | None =>
      Print.bold("knotc") |> Print.fmt("  %s <command> ...") |> print_endline;

      Print.bold("\nCOMMANDS\n") |> print_endline;

      _print_cmds(RunCmd.commands);
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
    Opt.create(
      ~alias="c",
      ~default=String(auto_config_file |?: ConfigFile.name),
      "config",
      String(
        Filename.resolve
        % (
          path => {
            if (!Sys.file_exists(path)) {
              path
              |> Print.fmt("unable to find the specified config file: %s")
              |> panic;
            };

            static := Some(ConfigFile.read(path));
            config_file := Some(path);
          }
        ),
      ),
      "set the location of the knot config file",
    ),
    debug_opt,
    color_opt,
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
        _print_cmds(RunCmd.commands);
        print_newline();

        panic("must provide a command");
      }
    );

  let root_dir = get_root_dir(static^);
  find_static_config(root_dir);

  switch (config_file^) {
  | Some(path) =>
    path
    |> Filename.relative_to(Sys.getcwd())
    |> Print.bold
    |> Log.info("found config file: %s")
  | None => Log.warn("no config file found")
  };

  let global_cfg = {
    root_dir,
    debug: get_debug(static^),
    color: get_color(static^),
    name:
      switch (static^) {
      | Some({name: Some(name)}) => name
      | _ => Filename.basename(root_dir)
      },
  };

  (global_cfg, resolve(static^, global_cfg));
};
