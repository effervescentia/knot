/* expressions */
module KSX = KSX.Interface;
module Style = KStyle.Plugin;
module Import = KImport.Plugin;
module Lambda = KLambda.Plugin;
module Primitive = KPrimitive.Plugin;
module Statement = KStatement.Plugin;
module Expression = KExpression.Plugin;
module Declaration = KDeclaration.Plugin;
module ModuleStatement = KModuleStatement.Plugin;

/* types */
module TypeStatement = KTypeStatement.Plugin;
module TypeExpression = KTypeExpression.Plugin;
module TypeDefinition = KTypeDefinition.Plugin;

type program_t('typ) =
  list(ModuleStatement.node_t(Declaration.node_t('typ)));
