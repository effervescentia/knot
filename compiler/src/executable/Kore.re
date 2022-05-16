include Knot.Kore;
include Reference;

module Compiler = Compile.Compiler;

type fatal_error_t =
  | MissingCommand
  | InvalidArgument(string, string)
  | UnexpectedArgument(string)
  | UnknownTarget(string)
  | InvalidConfigFile(string, string);

exception FatalError(fatal_error_t);

type path_t = {
  relative: string,
  absolute: string,
};

let binary_name = "knotc";
let config_file_name = ".knot.yml";

let config_key = "config";
let name_key = "name";
let cwd_key = "cwd";
let root_dir_key = "root_dir";
let source_dir_key = "source_dir";
let out_dir_key = "out_dir";
let target_key = "target";
let entry_key = "entry";
let debug_key = "debug";
let color_key = "color";
let fix_key = "fix";
let fail_fast_key = "fail_fast";
let log_imports_key = "log_imports";
let port_key = "port";

let is_ci_env =
  switch (Sys.getenv_opt("CI")) {
  | Some("")
  | None => false
  | Some(_) => true
  };

let fatal = err => raise(FatalError(err));

let target_of_string = x =>
  Target.of_string(x) |!: (() => UnknownTarget(x) |> fatal);
