include Knot.Core;

module Debug = Knot.Debug;

type config = {
  main: string,
  is_server: bool,
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

exception MissingRootDirectory;
exception InvalidPathFormat(string);
exception ModuleDoesNotExist(string, string);
exception InvalidEntryPoint(string);
exception EntryPointOutsideBuildContext(string);
