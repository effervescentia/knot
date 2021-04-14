/**
 Utilities for the "lsp" command.
 */
open Kore;
open Lwt.Infix;

module Server = LanguageServer.Server;

type config_t = unit;

let cmd = () => {
  let (port_opt, get_port) = ConfigOpt.port();

  Cmd.create(lsp_key, [port_opt], (static, global) => ());
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(global, lsp_key, []);

  Server.start();
};
