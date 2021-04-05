/**
 Utilities for the "build" command.
 */
open Kore;

type config_t = {
  target: Target.t,
  out_dir: string,
  entry: Namespace.t,
  fail_fast: bool,
};

let cmd = () => {
  let (out_dir_opt, get_out_dir) = ConfigOpt.out_dir();
  let (target_opt, get_target) = ConfigOpt.target();
  let (entry_opt, get_entry) = ConfigOpt.entry();
  let (fail_fast_opt, get_fail_fast) = ConfigOpt.fail_fast();

  Cmd.create(
    build_key, [out_dir_opt, target_opt, entry_opt], (static, global) =>
    {
      target: get_target(static),
      out_dir: get_out_dir(static, global.root_dir),
      entry: get_entry(static, global.root_dir, global.source_dir),
      fail_fast: get_fail_fast(static),
    }
  );
};

let run = (global: global_t, ~report=Reporter.panic, config: config_t) => {
  Cmd.log_config(
    global,
    build_key,
    [
      (out_dir_key, config.out_dir),
      (target_key, config.target |> Target.to_string),
      (entry_key, config.entry |> Namespace.to_string),
    ],
  );

  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: global.source_dir,
        fail_fast: config.fail_fast,
      },
    );

  compiler |> Compiler.compile(config.target, config.out_dir, config.entry);
  compiler |> Compiler.teardown;
};
