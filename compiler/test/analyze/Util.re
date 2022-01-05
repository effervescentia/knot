open Kore;
open Reference;

module Analyzer = Analyze.Analyzer;

include TestLibrary.Util;

let create_scope = name =>
  Scope.create(Namespace.of_string(name), ignore, Range.zero);
let create_id = Identifier.of_string % CommonUtil.as_raw_node;
