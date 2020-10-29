open Globals;

let _generate_common_module_import = (module_name, module_import) =>
  Printf.sprintf("var %s = require(\"%s\");", module_import, module_name)
and _generate_common_named_import = module_import =>
  Knot.Print.sequential(
    ~separator="",
    fun
    | (export_name, None) =>
      Printf.sprintf(
        "var %s = %s.%s;",
        export_name,
        module_import,
        export_name,
      )
    | (export_name, Some(alias_name)) =>
      Printf.sprintf(
        "var %s = %s.%s;",
        alias_name,
        module_import,
        export_name,
      ),
  );

let generate_import = (module_name, module_import, named_imports) => {
  let module_import_name =
    switch (module_import) {
    | Some(name) => name
    | None => Util.get_safe_variable_name(module_name)
    };

  _generate_common_module_import(module_name, module_import_name)
  ++ _generate_common_named_import(module_import_name, named_imports);
};

let generate_export = (name, alias) =>
  Printf.sprintf("exports.%s = %s;", Knot.Option.default(name, alias), name);
