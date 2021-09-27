include Knot.Kore;
include Reference;

module Compiler = Compile.Compiler;

let build_key = "build";
let watch_key = "watch";
let format_key = "format";
let lint_key = "lint";
let lsp_key = "lsp";
let bundle_key = "bundle";
let develop_key = "develop";

let name_key = "name";
let root_dir_key = "root_dir";
let source_dir_key = "source_dir";
let out_dir_key = "out_dir";
let target_key = "target";
let entry_key = "entry";
let debug_key = "debug";
let color_key = "color";
let fix_key = "fix";
let fail_fast_key = "fail_fast";
let port_key = "port";

let is_ci =
  switch (Sys.getenv_opt("CI")) {
  | Some("")
  | None => false
  | Some(_) => true
  };

let default_config = Knot.Config.defaults(is_ci);

type global_t = {
  debug: bool,
  color: bool,
  name: string,
  root_dir: string,
};

let panic = (err: string) => {
  Log.fatal("%s", err);

  exit(2);
};

let target_of_string = x =>
  Target.of_string(x)
  |!: (() => x |> Fmt.str("unknown target: '%s'") |> panic);
