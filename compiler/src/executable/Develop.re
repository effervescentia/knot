/**
 Utilities for the "develop" command.
 */
open Kore;

type config_t = {port: int};

let cmd = () => {
  let (port_opt, get_port) = ConfigOpt.port();

  Cmd.create(develop_key, [port_opt], (static, _) =>
    {port: get_port(static)}
  );
};

let run = (global: global_t, config: config_t): Lwt.t(unit) => {
  Cmd.log_config(
    global,
    develop_key,
    [(port_key, string_of_int(config.port))],
  );

  Lwt.return();
};
