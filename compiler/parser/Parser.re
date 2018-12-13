open Parsing;

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

let prog = input => (Module.body << eof())(input);

let parse = input =>
  switch (prog(input)) {
  | Some((res, _)) => Some(res)
  | None => None
  };