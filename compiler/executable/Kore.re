include Knot.Globals;

module Debug = Knot.Debug;
module FileStream = Knot.FileStream;
module TokenStream = KnotLex.TokenStream;
module Parser = KnotParse.Parser;
module Analyzer = KnotAnalyze.Analyzer;
module Scope = KnotAnalyze.Scope;
module Generator = KnotGen.Generator;

type inner_configuration = {
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
