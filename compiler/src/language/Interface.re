/* expressions */
module KSX = KSX.Interface;
module Style = KStyle.Interface;
module Lambda = KLambda.Interface;
module Primitive = KPrimitive.Interface;
module Statement = KStatement.Interface;
module Expression = KExpression.Interface;
module Declaration = KDeclaration.Interface;
module ModuleStatement = KModuleStatement.Interface;

/* types */
module TypeStatement = KTypeStatement.Interface;
module TypeExpression = KTypeExpression.Interface;
module TypeDefinition = KTypeDefinition.Interface;

type program_t('typ) =
  list(
    KModuleStatement.Interface.node_t(KDeclaration.Interface.node_t('typ)),
  );
