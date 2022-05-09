/**
 Utilities for the "lang_serve" command.
 */
open Kore;

module Server = LanguageServer.Server;

type config_t = unit;

let command_key = "lang_serve";

let command = () => {
  Command.create(command_key, [], (_, _) => ());
};

let run = (global: Config.global_t, config: config_t) => {
  Util.log_config(global, command_key, []);

  Server.start(folder =>
    ConfigFile.find(folder) |?> ConfigFile.read |?: ConfigFile.defaults
  );
};
