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
  port: int,
};

let defaults = (is_ci: bool) => {
  name: None,
  root_dir: Sys.getcwd(),
  source_dir: "src",
  out_dir: "build",
  entry: "main.kn",
  target: None,
  debug: false,
  color: !is_ci,
  fix: false,
  fail_fast: false,
  port: 1337,
};
