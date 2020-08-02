open Globals;

let module_alias = M.as_ >> M.identifier ==> (alias => Some(alias));
let module_import =
  M.asterisk >> M.as_ >> M.identifier ==> (name => [ModuleExport(name)]);
let main_import = M.identifier ==> (name => [MainExport(name)]) |= [];
let named_import =
  M.identifier
  >>= (name => module_alias |= None ==> (alias => NamedExport(name, alias)));
let named_imports =
  M.comma_separated(named_import)
  |> M.braces
  >>= (
    imports =>
      module_alias
      |= None
      ==> (
        fun
        | Some(alias) => [ModuleExport(alias), ...imports]
        | None => imports
      )
  );
let main_and_named =
  main_import
  >>= (main => M.comma >> named_imports |= [] ==> (named => main @ named));
let import_stmts =
  module_import <|> named_imports <|> main_and_named ==> List.map(no_ctx);

let stmt =
  M.import
  >> import_stmts
  >>= (imports => M.from >> M.string ==> (s => Import(s, imports)))
  |> M.terminated;
