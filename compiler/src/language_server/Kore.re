include Knot.Kore;

module Compiler = Compile.Compiler;
module JSON = Yojson.Basic;
module ModuleTable = AST.ModuleTable;
module Namespace = Reference.Namespace;
module ScopeTree = Compile.ScopeTree;

let file_schema = "file://";
