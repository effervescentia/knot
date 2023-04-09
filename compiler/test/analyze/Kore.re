include Knot.Kore;
include Test.Infix;
include ModuleAliases;

module ModuleStatement = {
  include KModuleStatement;
  include KModuleStatement.Plugin;
  include KModuleStatement.Interface;
};
module Import = {
  include KImport;
  include KImport.Plugin;
  include KImport.Interface;
};
module Declaration = {
  include KDeclaration;
  include KDeclaration.Plugin;
  include KDeclaration.Interface;
};
module Expression = {
  include KExpression;
  include KExpression.Plugin;
  include KExpression.Interface;
};
module Statement = {
  include KStatement;
  include KStatement.Plugin;
  include KStatement.Interface;
};
module KSX = {
  include KSX;
  include KSX.Plugin;
  include KSX.Interface;
};
module Primitive = {
  include KPrimitive;
  include KPrimitive.Plugin;
  include KPrimitive.Interface;
};
module TypeDefinition = {
  include KTypeDefinition;
  include KTypeDefinition.Plugin;
  include KTypeDefinition.Interface;
};
module TypeExpression = {
  include KTypeExpression;
  include KTypeExpression.Plugin;
  include KTypeExpression.Interface;
};
module TypeStatement = {
  include KTypeStatement;
  include KTypeStatement.Plugin;
  include KTypeStatement.Interface;
};
