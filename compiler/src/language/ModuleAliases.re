include KExpression.ModuleAliases;
include KDeclaration.ModuleAliases;

module Lambda = KLambda.Plugin;
module Expression = KExpression.Plugin;

/* statement */
module Statement = KStatement.Plugin;

/* module */
module Import = KImport.Plugin;
module Declaration = KDeclaration.Plugin;
module ModuleStatement = KModuleStatement.Plugin;

/* type */
module TypeStatement = KTypeStatement.Plugin;
module TypeExpression = KTypeExpression.Plugin;
module TypeDefinition = KTypeDefinition.Plugin;
