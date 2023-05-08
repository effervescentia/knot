include ModuleAliases;

type program_t('typ) =
  list(ModuleStatement.node_t(Declaration.node_t('typ)));
