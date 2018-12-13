type import_target =
  | MainExport(option(string))
  | NamedExport(string, option(string));

type declaration =
  | /**
   * @param
   */
    ConstDecl(string)
  | StateDecl(string)
  | ViewDecl(string)
  | FunctionDecl(string);

type module_ =
  | Statements(list(module_))
  | Import(string, list(import_target))
  | Declaration(declaration);