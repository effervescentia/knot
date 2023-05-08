include Knot.Kore;

module Compiler = Compile.Compiler;
module JSON = Yojson.Basic;
module ModuleTable = AST.ModuleTable;
module Namespace = Reference.Namespace;
module ScopeTree = Compile.ScopeTree;
module Declaration = KDeclaration.Plugin;
module ModuleStatement = KModuleStatement.Plugin;

let file_schema = "file://";
