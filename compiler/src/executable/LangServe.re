/**
 Utilities for the "lang_serve" command.
 */
open Kore;

module Server = LanguageServer.Server;

type config_t = unit;

let command_key = "lang_serve";

let command = () => {
  Command.create(command_key, [], (_, _, _) => ());
};

let _find_config = (global: Config.global_t, folder) => {
  let default_config = ConfigFile.get_defaults(~color=global.color, ());
  let config =
    switch (ConfigFile.find(folder)) {
    | Some(file) =>
      switch (ConfigFile.read(file)) {
      | Ok(config) => config
      | Error(err) =>
        err
        |> ConfigFile.describe_error
        |> Log.warn("cannot use config file '%s'\n%s", file);

        default_config;
      }
    | None => default_config
    };

  let root_dir = Filename.resolve(~cwd=folder, config.root_dir);
  let source_dir = Filename.resolve(~cwd=root_dir, config.source_dir);
  let out_dir = Filename.resolve(~cwd=root_dir, config.out_dir);

  {...config, root_dir, source_dir, out_dir};
};

let extract_config = (config: config_t) => [];

let run = (global: Config.global_t, config: config_t) => {
  Util.log_config(global, command_key, extract_config(config));

  Log.info("%s", "starting language server" |> ~@Fmt.warn_str);

  global |> _find_config |> Server.start(global.stdlib);
};
