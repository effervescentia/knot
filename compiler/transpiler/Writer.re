open Globals;

module Generator = KnotGenerate.Generator;

let clean_dir = dir =>
  if (Sys.file_exists(dir)) {
    FileUtil.clean_directory(dir);
  };

let write =
    (
      {paths: {build_dir, source_dir, root_dir}, module_type},
      path,
      (module_name, ast),
    ) => {
  let path = Printf.sprintf("%s.js", path) |> Filename.concat(build_dir);

  Filename.dirname(path) |> FileUtil.mkdir_p;

  FileUtil.relative_path(root_dir, path)
  |> Log.info("%s  %s (%s)", Emoji.printer, module_name);

  let out_channel = open_out(path);
  let write_out = output_string(out_channel);

  Generator.generate(
    write_out,
    {
      to_module_name: FileUtil.normalize_module(source_dir),
      to_import_statement:
        switch (module_type) {
        | Common => Generator.generate_common_import_statement
        | ES6 => Generator.generate_es6_import_statement
        },
      to_export_statement:
        switch (module_type) {
        | Common => Generator.generate_common_export_statement
        | ES6 => Generator.generate_es6_export_statement
        },
    },
    fst(ast),
  );

  close_out(out_channel);
};
