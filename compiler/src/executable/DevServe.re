/**
 Utilities for the "dev_serve" command.
 */
open Kore;

type config_t = {port: int};

let command_key = "dev_serve";

let command = () => {
  let (port_arg, get_port) = Arg_Port.create();

  Command.create(
    command_key,
    [port_arg],
    (static, _) => {
      let port = get_port(static);

      {port: port};
    },
  );
};

let run = (global: Config.global_t, config: config_t): Lwt.t(unit) => {
  Util.log_config(
    global,
    command_key,
    [(port_key, string_of_int(config.port))],
  );

  Lwt.return();
};
