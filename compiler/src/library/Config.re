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
};

let default_source_dir = "src";
let default_out_dir = "build";
let default_entry = "main.kn";
let default_port = 1337;

let defaults = (is_ci_env: bool) => {
  name: None,
  root_dir: Sys.getcwd(),
  source_dir: default_source_dir,
  out_dir: default_out_dir,
  entry: default_entry,
  target: None,
  debug: false,
  color: !is_ci_env,
  fix: false,
  fail_fast: false,
  log_imports: false,
  port: default_port,
};
