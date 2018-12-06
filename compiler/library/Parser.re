open Angstrom;

type declaration =
  | ConstDecl
  | StateDecl
  | ViewDecl
  | FunctionDecl;

type module_ =
  | Statements(list(module_))
  | Declaration(declaration);