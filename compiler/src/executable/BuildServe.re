/**
 Utilities for the "build_serve" command.
 */
open Kore;

module Server = BuildServer.Server;

type config_t = {
  source_dir: string,
  target: Target.t,
};

let cmd = () => {
  let (source_dir_opt, get_source_dir) = ConfigOpt.source_dir();
  let (target_opt, get_target) = ConfigOpt.target();

  Cmd.create(
    build_serve_key,
    [source_dir_opt, target_opt],
    (static, global) => {
      let source_dir = get_source_dir(static, global.root_dir);

      {target: get_target(static), source_dir};
    },
  );
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(
    global,
    build_serve_key,
    [(target_key, config.target |> ~@Target.pp)],
  );

  Server.start({
    name: global.name,
    root_dir: global.root_dir,
    source_dir: config.source_dir,
    target: config.target,
  });
};
