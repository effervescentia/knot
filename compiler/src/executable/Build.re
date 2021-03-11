/**
 Utilities for the "build" command.
 */
open Kore;

type config_t = {
  target: Target.t,
  out_dir: string,
};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();
  let (target_opt, get_target) = Opt.Shared.target();

  let resolve = () => {target: get_target(), out_dir: get_out_dir()};

  ("build", [out_dir_opt, target_opt], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t) => {
  let compiler = Compiler.create(~catch=print_errs % panic, cfg);

  compiler |> Compiler.compile;
  compiler |> Compiler.teardown;
};
