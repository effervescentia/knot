/**
 Utilities for the "build_serve" command.
 */
open Kore;
open Lwt.Infix;

module Server = LanguageServer.Server;

type config_t = unit;

let cmd = () => {
  Cmd.create(build_serve_key, [], (_, _) => ());
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(global, build_serve_key, []);

  Server.start(folder =>
    ConfigFile.find(folder) |?> ConfigFile.read |?: default_config
  );
};
