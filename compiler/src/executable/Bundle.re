/**
 Utilities for the "bundle" command.
 */
open Kore;

type config_t = {out_dir: string};

let cmd = () => {
  let (out_dir_opt, get_out_dir) = ConfigOpt.out_dir();

  Cmd.create(bundle_key, [out_dir_opt], (static, global) =>
    {out_dir: get_out_dir(static, global.root_dir)}
  );
};

let run = (global: global_t, config: config_t) => {
  Cmd.log_config(global, bundle_key, [(out_dir_key, config.out_dir)]);

  ();
};
