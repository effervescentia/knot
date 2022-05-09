/**
 Parse command line args into config.
 */
open Kore;

module Task = Executable.Task;
module ConfigFile = Executable.ConfigFile;
module Arguments = Executable.Arguments;
module Usage = Executable.Usage;

let __commands = [
  Task.build,
  Task.watch,
  Task.format,
  Task.lint,
  Task.bundle,
  Task.dev_serve,
  Task.lang_serve,
  Task.build_serve,
];

let _read_config = file =>
  switch (ConfigFile.read(file)) {
  | Ok(config) =>
    file
    |> Filename.relative_to(Sys.getcwd())
    |> Log.info("found config file: %a", ~$Fmt.bold_str);

    config;

  | Error(err) =>
    err
    |> ConfigFile.describe_error
    |> Fmt.str("cannot to use config file: %a\n%s", Fmt.bold_str, file)
    |> panic
  };

let _expand_args = List.map(Argument.expand) % List.flatten;

let run = (): (Config.global_t, Task.t) => {
  Fmt.color := !is_ci_env;

  let need_help = ref(false);
  let command = ref(None);
  let config_file = ref(None);
  let static_config = ref(None);

  /* arguments */
  let (config_file_arg, get_config_file) =
    Arguments.config_file(
      ~default=ConfigFile.find(ConfigFile.defaults.root_dir),
      (),
    );
  let (debug_arg, get_debug) = Arguments.debug();
  let (color_arg, get_color) = Arguments.color();
  let help_arg =
    Argument.create(
      "help",
      Unit(() => need_help := true),
      "display this list of options",
    );
  let global_arguments = [config_file_arg, debug_arg, color_arg, help_arg];
  /* replaces the inbuilt -help arg */
  let hidden_arguments = [("-help", Arg.Unit(() => need_help := true), "")];
  let initial_arguments = _expand_args(global_arguments) @ hidden_arguments;
  let arguments = ref(initial_arguments);

  /* parse arguments */
  Arg.parse_dynamic(
    arguments,
    arg =>
      if (command^ == None) {
        switch (__commands |> List.find_opt(Command.(cmd => cmd.name == arg))) {
        | Some(cmd) =>
          command := Some(cmd);
          arguments := _expand_args(cmd.arguments) @ initial_arguments;

        | _ => arg |> Fmt.str("unexpected argument: %s") |> panic
        };
      },
    Fmt.str("%s [options] <command>", binary_name),
  );

  get_config_file()
  |> Option.iter(path => {
       let config = _read_config(path);

       config_file := Some(path);
       static_config := Some(config);
     });

  /* print usage information if any help flags passed */
  if (need_help^) {
    Fmt.pr("%a", Usage.pp, (command^, static_config^, global_arguments));

    exit(2);
  };

  if (config_file^ == None) {
    Log.warn("no config file found");
  };

  /* handle command if found */
  switch (command^) {
  | Some({resolve: resolve_command_config}) =>
    let working_dir =
      switch (config_file^) {
      | Some(path) => Filename.dirname(path)
      | None => Sys.getcwd()
      };

    let global_config =
      Config.{
        debug: get_debug(static_config^),
        color: get_color(static_config^),
        name:
          switch (static_config^) {
          | Some({name: Some(name)}) => name
          | _ => Filename.basename(working_dir)
          },
        working_dir,
      };

    Fmt.color := global_config.color;

    (global_config, resolve_command_config(static_config^, global_config));

  | None =>
    Fmt.pr("%a@,", Usage.pp_command_list, Usage.commands);

    panic("must provide a command");
  };
};
