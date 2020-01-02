include Knot.Core;
include Exception;

module Debug = Knot.Debug;
module Compiler = KnotCompile.Compiler;

type module_type =
  | ES6
  | Common;

type config = {
  main: string,
  module_type,
  is_server: bool,
  is_debug: bool,
  port: int,
  paths: paths_config,
}
and paths_config = {
  config_file: string,
  root_dir: string,
  source_dir: string,
  build_dir: string,
  module_dir: string,
};
