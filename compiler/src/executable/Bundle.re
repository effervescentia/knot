/**
 Utilities for the "bundle" command.
 */
open Kore;

type config_t = {out_dir: string};

let mode = () => {
  let (out_dir_opt, get_out_dir) = Opt.Shared.out_dir();

  Mode.create("bundle", [out_dir_opt], (static, global) =>
    {out_dir: get_out_dir(static, global.root_dir)}
  );
};

let run = (global: global_t, config: config_t) => {
  Log.info("running 'bundle' command");
  Log.debug(
    "bundle config: %s",
    [
      ("name", global.name),
      ("root_dir", global.root_dir),
      ("source_dir", global.source_dir),
      ("out_dir", config.out_dir),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );

  ();
};
