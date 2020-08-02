open Globals;

let _generate_es6_module_import = (module_name, module_import) =>
  switch (module_import) {
  | Some(name) =>
    Printf.sprintf("import * as %s from \"%s\";", name, module_name)
  | _ => ""
  }
and _generate_es6_named_import = (module_name, named_imports) =>
  gen_list(
    fun
    | (export_name, Some(alias_name)) =>
      Printf.sprintf("%s as %s", export_name, alias_name)
    | (export_name, None) => export_name,
    named_imports,
  )
  |> (
    s =>
      s == ""
        ? "" : Printf.sprintf("import { %s } from \"%s\";", s, module_name)
  );

let generate_import = (module_name, module_import, named_imports) =>
  _generate_es6_module_import(module_name, module_import)
  ++ _generate_es6_named_import(module_name, named_imports);

let generate_export = (name, alias) =>
  (
    switch (alias) {
    | Some(alias_name) => Printf.sprintf(" as %s", alias_name)
    | None => ""
    }
  )
  |> Printf.sprintf("export { %s%s };", name);
