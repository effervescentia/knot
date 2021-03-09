/**
 Utilities for the "bundle" command.
 */
open Kore;

type config_t = {out_dir: string};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();

  let resolve = () => {out_dir: get_out_dir()};

  ("bundle", [out_dir_opt], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t) => {
  ();
};
