/**
 Utilities for the "lint" command.
 */
open Kore;

type config_t = {
  root_dir: string,
  fix: bool,
};

let command_key = "lint";

let command = () => {
  let (root_dir_arg, get_root_dir) = Arg_RootDir.create();
  let (fix_arg, get_fix) = Arg_Fix.create();

  Command.create(
    command_key,
    [root_dir_arg, fix_arg],
    (static, global) => {
      let root_dir = get_root_dir(static, global.working_dir);
      let fix = get_fix(static);

      {root_dir, fix};
    },
  );
};

let run = (global: Config.global_t, config: config_t) => {
  Util.log_config(
    global,
    command_key,
    [
      (root_dir_key, config.root_dir),
      (fix_key, string_of_bool(config.fix)),
    ],
  );

  ();
};
