/**
 Utilities for the "lsp" command.
 */
open Kore;
open Lwt.Infix;

module Server = LanguageServer.Server;

type config_t = unit;

let cmd = () => {
  Cmd.create(lsp_key, [], (_, _) => ());
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(global, lsp_key, []);

  Server.start(folder =>
    ConfigFile.find(folder) |?> ConfigFile.read |?: default_config
  );
};
