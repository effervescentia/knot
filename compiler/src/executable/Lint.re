/**
 Utilities for the "lint" command.
 */
open Kore;

type config_t = {fix: bool};

let fix_opt = (~default=false, ()) => {
  let value = ref(default);
  let opt =
    Opt.create(
      ~alias="f",
      ~default=Bool(default),
      ~from_config=cfg => Some(Bool(cfg.fix)),
      "fix",
      Arg.Set(value),
      "automatically apply fixes",
    );
  let resolve = () => value^;

  (opt, resolve);
};

let mode = () => {
  let (fix_opt, get_fix) = fix_opt();

  Mode.create("lint", [fix_opt], (static, global) => {fix: get_fix()});
};

let run = (global: global_t, config: config_t) => {
  Log.info("running 'lint' command");
  Log.debug(
    "lint config: %s",
    [
      ("name", global.name),
      ("root_dir", global.root_dir),
      ("source_dir", global.source_dir),
      ("fix", config.fix |> string_of_bool),
    ]
    |> List.to_seq
    |> Hashtbl.of_seq
    |> Hashtbl.to_string(Functional.identity, Functional.identity)
    |> Pretty.to_string,
  );

  ();
};
