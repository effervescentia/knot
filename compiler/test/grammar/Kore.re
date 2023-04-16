include Knot.Kore;
include Test.Infix;
include ModuleAliases;

module KSX = {
  include KSX;
  include KSX.Plugin;
  include KSX.Interface;
};

module BinaryOperator = KBinaryOperator.Plugin;
module UnaryOperator = KUnaryOperator.Plugin;
module ParseContext = AST.ParseContext;
module Namespace = Reference.Namespace;
module InputStream = File.InputStream;
module SymbolTable = AST.SymbolTable;
module Export = Reference.Export;
module Import = KImport.Plugin;
module Primitive = KPrimitive.Plugin;
module Statement = KStatement.Plugin;
module Expression = KExpression.Plugin;
module Declaration = KDeclaration.Plugin;
module TypeExpression = KTypeExpression.Plugin;
module TypeStatement = KTypeStatement.Plugin;
module TypeDefinition = KTypeDefinition.Plugin;
module ModuleStatement = KModuleStatement.Plugin;
module ExportKind = ModuleStatement.ExportKind;
module Parser = Parse.Parser;
module Type = AST.Type;
