/**
 * Utilities for the "lsp" command.
 */
open Kore;

type config_t = {port: int};

let mode = () => {
  let (port_opt, get_port) = Opt.Shared.port();

  let resolve = () => {port: get_port()};

  ("lsp", [port_opt], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t): Lwt.t(unit) => {
  Lwt.return();
};
