/**
 Parse command line args into config.
 */
open Kore;

let _pp_section = (pp_value: Fmt.t('a)): Fmt.t((string, 'a)) =>
  Fmt.(
    (ppf, (label, value)) =>
      pf(ppf, "%a@,%a", bold_str, label, indented(pp_value), value)
  );

let _pp_command_list = (ppf, cmds) => {
  let offset =
    cmds
    |> List.map(((x, _)) => String.length(x))
    |> List.fold_left((acc, len) => len > acc ? len : acc, 0)
    |> (+)(2);

  Fmt.(
    list(
      ~sep=Sep.trailing_newline,
      (ppf, (cmd, desc)) =>
        pf(ppf, "%a%s", bold(ppf => pf(ppf, "%-*s", offset)), cmd, desc),
      ppf,
      cmds,
    )
  );
};

let _pp_opt_list = cfg => Fmt.(list(~sep=Sep.trailing_newline, Opt.pp(cfg)));

let _pp_sub_command: Fmt.t(Cmd.t('a)) =
  Fmt.(
    (ppf, {name}) =>
      pf(ppf, "  %a [options]", bold(ppf => pf(ppf, "knotc %s")), name)
  );

let _pp_cmd_usage = (static_cfg): Fmt.t(option(Cmd.t(RunCmd.t))) =>
  Fmt.(
    ppf =>
      fun
      | None =>
        pf(
          ppf,
          "  %a <command> ...@,@,%a",
          bold_str,
          "knotc",
          _pp_section(_pp_command_list),
          ("COMMANDS", RunCmd.commands),
        )

      | Some(Cmd.{name, opts: command_opts} as cmd) =>
        pf(
          ppf,
          "%a@,%a",
          _pp_sub_command,
          cmd,
          ppf =>
            fun
            | [] => nop(ppf, ())
            | opts =>
              pf(
                ppf,
                "@,%a",
                _pp_section(_pp_opt_list(static_cfg)),
                ("COMMAND OPTIONS", opts),
              ),
          command_opts,
        )
  );

let rec pp_usage:
  Fmt.t((option(Cmd.t(RunCmd.t)), option(Config.t), list(Opt.t))) =
  Fmt.(
    (ppf, (cmd, static_cfg, opts)) =>
      root(
        ppf =>
          pf(
            ppf,
            "%a@,%a",
            _pp_cmd_usage(static_cfg),
            cmd,
            _pp_section(_pp_opt_list(static_cfg)),
          ),
        ppf,
        ("OPTIONS", opts),
      )
  );

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

    Fmt.color := !is_ci;

    Fmt.pr("%a", pp_usage, (cmd^, static^, global_opts()));

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
              |> Fmt.str("unable to find the specified config file: %s")
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
        | (None, key) when key == bundle_key => set_cmd(RunCmd.bundle)
        | (None, key) when key == dev_serve_key => set_cmd(RunCmd.dev_serve)
        | (None, key) when key == lang_serve_key =>
          set_cmd(RunCmd.lang_serve)
        | (None, key) when key == build_serve_key =>
          set_cmd(RunCmd.build_serve)
        | _ => Fmt.str("unexpected argument: %s", x) |> panic
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
        Fmt.pr("%a@,", _pp_command_list, RunCmd.commands);

        panic("must provide a command");
      }
    );

  let root_dir = get_root_dir(static^);
  find_static_config(root_dir);

  switch (config_file^) {
  | Some(path) =>
    path
    |> Filename.relative_to(Sys.getcwd())
    |> Log.info("found config file: %a", ~$Fmt.bold_str)
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
