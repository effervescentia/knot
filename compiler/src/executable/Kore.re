include Knot.Kore;
include Reference;

module Target = Generate.Target;
module Compiler = Compile.Compiler;

let build_key = "build";
let watch_key = "watch";
let format_key = "format";
let lint_key = "lint";
let lsp_key = "lsp";
let bundle_key = "bundle";
let develop_key = "develop";

let root_dir_key = "root_dir";
let source_dir_key = "source_dir";
let out_dir_key = "out_dir";
let target_key = "target";
let entry_key = "entry";
let debug_key = "debug";
let color_key = "color";
let fix_key = "fix";
let port_key = "port";

let is_ci =
  switch (Sys.getenv_opt("CI")) {
  | Some("")
  | None => false
  | Some(_) => true
  };

type static_t = {
  root_dir: string,
  source_dir: string,
  out_dir: string,
  target: option(Target.t),
  entry: string,
  debug: bool,
  color: bool,
  fix: bool,
  port: int,
};

let defaults = {
  root_dir: Sys.getcwd(),
  source_dir: "src",
  out_dir: "build",
  entry: "main.kn",
  target: None,
  debug: false,
  color: !is_ci,
  fix: false,
  port: 1337,
};

type global_t = {
  debug: bool,
  name: string,
  root_dir: string,
  source_dir: string,
};

let panic = (err: string) => {
  Log.fatal("%s", err);

  exit(2);
};

let target_of_string = x =>
  Target.of_string(x)
  |!: (() => x |> Print.fmt("unknown target: '%s'") |> panic);
