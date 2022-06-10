/**
 Parse command line args into config.
 */
open Kore;

let _commands = () => [
  Task.build(),
  Task.watch(),
  Task.format(),
  Task.lint(),
  Task.bundle(),
  Task.dev_serve(),
  Task.lang_serve(),
  Task.build_serve(),
];

let _read_config = (defaults, cwd, file) =>
  switch (ConfigFile.read(~defaults, file)) {
  | Ok(config) =>
    Log.info("found config file %s", file |> ~@Fmt.relative_path(cwd));

    config;

  | Error(err) =>
    InvalidConfigFile(file, ConfigFile.describe_error(err)) |> fatal
  };

let _expand_args = List.map(Argument.expand) % List.flatten;

let run =
    (~cwd=Sys.getcwd(), ~argv=Sys.argv, ~color=!is_ci_env, stdlib: string)
    : (Config.global_t, Task.t) => {
  let command = ref(None);
  let config_file = ref(None);
  let static_config = ref(None);
  let default_config = ConfigFile.get_defaults(~color, ());

  /* arguments */
  let (cwd_arg, get_cwd) = Arg_CWD.create(~default=cwd, ());
  let (config_file_arg, get_config_file) = Arg_Config.create();
  let (debug_arg, get_debug) = Arg_Debug.create();
  let (color_arg, get_color) = Arg_Color.create(~default=color, ());
  let ((help_arg, hidden_help_arg), get_help) = Arg_Help.create();
  let global_arguments = [
    cwd_arg,
    config_file_arg,
    debug_arg,
    color_arg,
    help_arg,
  ];
  /* replaces the inbuilt -help arg */
  let hidden_arguments = [hidden_help_arg];
  let initial_arguments = _expand_args(global_arguments) @ hidden_arguments;
  let arguments = ref(initial_arguments);

  /* parse arguments */
  Arg.parse_argv_dynamic(
    ~current=ref(0),
    argv,
    arguments,
    arg =>
      if (command^ == None) {
        switch (
          _commands() |> List.find_opt(Command.(cmd => cmd.name == arg))
        ) {
        | Some(cmd) =>
          command := Some(cmd);
          arguments := _expand_args(cmd.arguments) @ initial_arguments;

        | _ => UnexpectedArgument(arg) |> fatal
        };
      },
    Fmt.str("%s [options] <command>", binary_name),
  );

  let cwd_override = get_cwd();
  let cwd_temp = cwd_override |?: cwd;

  (
    switch (get_config_file(cwd_temp)) {
    | Some(_) as config => config
    | None => ConfigFile.find(cwd_temp)
    }
  )
  |> Option.iter(path => {
       let config = _read_config(default_config, cwd_temp, path);

       config_file := Some(path);
       static_config := Some(config);
     });

  let static_config = static_config^;

  /* print usage information if any help flags passed */
  if (get_help()) {
    Fmt.pr("%a", Usage.pp, (command^, static_config, global_arguments));

    exit(0);
  };

  /* handle command if found */
  switch (command^) {
  | Some({resolve: resolve_command_config}) =>
    if (config_file^ == None) {
      Log.warn("no config file found");
    };

    let working_dir =
      switch (config_file^) {
      | Some(path) => Filename.dirname(path)
      | _ => cwd_temp
      };

    let global_config =
      Config.{
        debug: get_debug(static_config),
        color: get_color(static_config),
        name:
          switch (static_config) {
          | Some({name: Some(name)}) => name
          | _ => Filename.basename(working_dir)
          },
        working_dir,
        stdlib,
      };

    Fmt.color := global_config.color;

    (global_config, resolve_command_config(static_config, global_config));

  | None =>
    Fmt.pr("%a@,", Usage.pp_command_list, Usage.commands);

    fatal(MissingCommand);
  };
};
