/**
 Utilities for the "lang_serve" command.
 */
open Kore;

module Server = LanguageServer.Server;

type config_t = unit;

let cmd = () => {
  Cmd.create(lang_serve_key, [], (_, _) => ());
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(global, lang_serve_key, []);

  Server.start(folder =>
    ConfigFile.find(folder) |?> ConfigFile.read |?: default_config
  );
};
