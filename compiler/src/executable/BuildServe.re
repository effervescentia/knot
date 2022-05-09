/**
 Utilities for the "build_serve" command.
 */
open Kore;

module Server = BuildServer.Server;

type config_t = {
  root_dir: string,
  source_dir: string,
  target: Target.t,
};

let command_key = "build_serve";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arguments.root_dir();
  let (source_dir_arg, get_source_dir) = Arguments.source_dir();
  let (target_arg, get_target) = Arguments.target();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg, target_arg],
    (static, global) => {
      let root_dir = get_root_dir(static);
      let source_dir = get_source_dir(static, root_dir);
      let target = get_target(static);

      {root_dir, source_dir, target};
    },
  );
};

let run = (global: Config.global_t, config: config_t) => {
  Util.log_config(
    global,
    command_key,
    [
      (root_dir_key, config.root_dir),
      (source_dir_key, config.source_dir),
      (target_key, config.target |> ~@Target.pp),
    ],
  );

  Server.start({
    name: global.name,
    root_dir: config.root_dir,
    source_dir: config.source_dir,
    target: config.target,
  });
};
