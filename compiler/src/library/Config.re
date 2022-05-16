open Infix;

module Fmt = Pretty.Formatters;

type t = {
  name: option(string),
  root_dir: string,
  source_dir: string,
  out_dir: string,
  target: option(Target.t),
  entry: string,
  debug: bool,
  color: bool,
  fix: bool,
  fail_fast: bool,
  log_imports: bool,
  port: int,
};

type global_t = {
  debug: bool,
  color: bool,
  name: string,
  working_dir: string,
};

let default_source_dir = "src";
let default_out_dir = "build";
let default_entry = "main.kn";
let default_port = 1337;

let defaults: t = {
  name: None,
  root_dir: ".",
  source_dir: default_source_dir,
  out_dir: default_out_dir,
  entry: default_entry,
  target: None,
  debug: false,
  color: false,
  fix: false,
  fail_fast: false,
  log_imports: false,
  port: default_port,
};

let pp: Fmt.t(t) =
  (ppf, config) =>
    (
      "Config",
      [
        ("name", config.name |> ~@Fmt.option(Fmt.string)),
        ("root_dir", config.root_dir),
        ("source_dir", config.source_dir),
        ("out_dir", config.out_dir),
        ("entry", config.entry),
        ("target", config.target |> ~@Fmt.option(Target.pp)),
        ("debug", string_of_bool(config.debug)),
        ("color", string_of_bool(config.color)),
        ("fix", string_of_bool(config.fix)),
        ("fail_fast", string_of_bool(config.fail_fast)),
        ("log_imports", string_of_bool(config.log_imports)),
        ("port", string_of_int(config.port)),
      ],
    )
    |> Fmt.struct_(Fmt.string, Fmt.string, ppf);
