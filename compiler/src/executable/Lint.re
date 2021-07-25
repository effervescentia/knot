/**
 Utilities for the "lint" command.
 */
open Kore;

type config_t = {fix: bool};

let cmd = () => {
  let (fix_opt, get_fix) = ConfigOpt.fix();

  Cmd.create(lint_key, [fix_opt], (static, _) => {fix: get_fix(static)});
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(
    global,
    lint_key,
    [(fix_key, string_of_bool(config.fix))],
  );

  ();
};
