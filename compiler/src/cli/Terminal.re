/**
 Parse command line args into config.
 */
open Kore;

module Task = Executable.Task;
module ConfigFile = Executable.ConfigFile;
module Arguments = Executable.Arguments;
module Usage = Executable.Usage;

let __tasks = [
  Task.build,
  Task.watch,
  Task.format,
  Task.lint,
  Task.bundle,
  Task.dev_serve,
  Task.lang_serve,
  Task.build_serve,
];

let read = (): (Config.global_t, Task.t) => {
  let auto_config_file = ConfigFile.find(ConfigFile.defaults.root_dir);
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

  let (debug_opt, get_debug) = Arguments.debug();
  let (color_opt, get_color) = Arguments.color();
  let (root_dir_opt, get_root_dir) = Arguments.root_dir();

  let find_static_config = root_dir =>
    if (config_file^ == None) {
      switch (File.Util.find_up(ConfigFile.file_name, root_dir)) {
      | Some(path) =>
        static := Some(ConfigFile.read(path));
        config_file := Some(path);
      | None => ()
      };
    };

  let rec usage = () => {
    find_static_config(get_root_dir(static^));

    Fmt.color := !is_ci_env;

    Fmt.pr("%a", Usage.pp, (cmd^, static^, global_arguments()));

    exit(2);
  }
  and global_arguments = () => [
    Argument.create(
      ~alias="c",
      ~default=String(auto_config_file |?: ConfigFile.file_name),
      "config",
      String(
        Filename.resolve
        % (
          path => {
            /* (
                 Filename.is_relative(path)
                   ? Filename.relative_to(get_root_dir(), path) : path
               ) */
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
    root_dir_opt,
    debug_opt,
    color_opt,
    Argument.create("help", Unit(usage), "display this list of options"),
  ];

  /* capture and replace the inbuilt -help arg */
  let hidden_opts = [("-help", Arg.Unit(usage), "")];

  let set_cmd = ({arguments} as m: Command.t('a)) => {
    cmd := Some(m);
    options :=
      (
        arguments
        @ global_arguments()
        |> List.map(Argument.expand)
        |> List.flatten
      )
      @ hidden_opts;
  };

  options :=
    (global_arguments() |> List.map(Argument.expand) |> List.flatten)
    @ hidden_opts;

  try(
    Arg.parse_dynamic(
      options,
      x =>
        if (cmd^ == None) {
          switch (__tasks |> List.find_opt(Command.(task => task.name == x))) {
          | Some(task) => set_cmd(task)
          | _ => Fmt.str("unexpected argument: %s", x) |> panic
          };
        },
      Fmt.str("%s [options] <command>", binary_name),
    )
  ) {
  | _ => ()
  };

  let Command.{resolve} =
    cmd^
    |!: (
      () => {
        Fmt.pr("%a@,", Usage.pp_command_list, Usage.commands);

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

  let global_cfg =
    Config.{
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
