/**
 Utilities for the "build" command.
 */
open Kore;

type config_t = {
  target: Target.t,
  out_dir: string,
  entry: Namespace.t,
};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();
  let (target_opt, get_target) = Opt.Shared.target();
  let (entry_opt, get_entry) = Opt.Shared.entry();

  Mode.create(
    "build", [out_dir_opt, target_opt, entry_opt], (static, global) =>
    {
      target: get_target(static),
      out_dir: get_out_dir(static, global.root_dir),
      entry: get_entry(static, global.root_dir, global.source_dir),
    }
  );
};

let run = (~report=print_errs % panic, global: global_t, config: config_t) => {
  Log.info("running 'build' command");
  Log.debug(
    "build config: %s",
    [
      ("name", global.name),
      ("root_dir", global.root_dir),
      ("source_dir", global.source_dir),
      ("out_dir", config.out_dir),
      ("target", config.target |> Target.to_string),
      ("entry", config.entry |> Namespace.to_string),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );

  let compiler =
    Compiler.create(
      ~report,
      {
        name: global.name,
        root_dir: global.root_dir,
        source_dir: global.source_dir,
      },
    );

  compiler |> Compiler.compile(config.target, config.out_dir, config.entry);
  compiler |> Compiler.teardown;
};
