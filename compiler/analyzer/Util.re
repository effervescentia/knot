open Core;

let extract_decl_name =
  fun
  | ConstDecl(name, _)
  | FunctionDecl(name, _, _)
  | StateDecl(name, _, _)
  | StyleDecl(name, _, _)
  | ViewDecl(name, _, _, _, _) => name;
