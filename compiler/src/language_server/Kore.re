include Knot.Kore;
include Reference;

module JSON = Yojson.Basic;
module Compiler = Compile.Compiler;
module ScopeTree = Compile.ScopeTree;

let file_schema = "file://";
