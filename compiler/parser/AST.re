type import_target =
  | MainExport(string)
  | ModuleExport(string)
  | NamedExport(string, option(string));

type declaration =
  | ConstDecl(string)
  | StateDecl(string)
  | ViewDecl(string)
  | FunctionDecl(string);

type module_ =
  | Statements(list(module_))
  | Import(string, list(import_target))
  | Declaration(declaration);