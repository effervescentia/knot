include Knot.Core;

module Analyzer = KnotAnalyze.Analyzer;
module Scope = KnotAnalyze.Scope;
module Parser = KnotParse.Parser;
module Loader = KnotFile.Loader;
module Debug = KnotDebug.Debug;
module FileUtil = KnotFile.Util;

type status =
  | Idle
  | Running
  | Complete
  | Failing(list((compilation_error, string, string)));

type link_descriptor = {
  target: string,
  absolute_path: string,
  relative_path: string,
  pretty_path: string,
};
