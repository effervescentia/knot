include Knot.Kore;
include Test.Infix;
include ModuleAliases;

module BinaryOperator = KBinaryOperator.Plugin;
module DotAccess = KDotAccess.Plugin;
module FunctionCall = KFunctionCall.Plugin;
module Style = KStyle.Plugin;
module UnaryOperator = KUnaryOperator.Plugin;
module View = KView.Plugin;
module Lambda = KLambda.Plugin;
module Import = KImport.Plugin;
module Primitive = KPrimitive.Plugin;
module Statement = KStatement.Plugin;
module Expression = KExpression.Plugin;
module Declaration = KDeclaration.Plugin;
module TypeExpression = KTypeExpression.Plugin;
module TypeStatement = KTypeStatement.Plugin;
module TypeDefinition = KTypeDefinition.Plugin;
module ModuleStatement = KModuleStatement.Plugin;

module KSX = {
  include KSX;
  include KSX.Plugin;
  include KSX.Interface;
};
