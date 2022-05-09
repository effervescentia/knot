/**
 Utilities for the "bundle" command.
 */
open Kore;

type config_t = {
  root_dir: string,
  source_dir: string,
  out_dir: string,
};

let command_key = "bundle";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arguments.root_dir();
  let (source_dir_arg, get_source_dir) = Arguments.source_dir();
  let (out_dir_arg, get_out_dir) = Arguments.out_dir();

  Command.create(
    command_key,
    [root_dir_arg, source_dir_arg, out_dir_arg],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let source_dir = get_source_dir(static, root_dir);
      let out_dir = get_out_dir(static, root_dir);

      {root_dir, source_dir, out_dir};
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
      (out_dir_key, config.out_dir),
    ],
  );

  ();
};
