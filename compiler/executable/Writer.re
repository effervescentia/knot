open Kore;

module Generator = KnotGenerate.Generator;
module CompilerUtil = KnotCompile.Util;

let clean_build_dir = () => {
  let {paths: {build_dir}} = Config.get();
  Config.root_path(build_dir) |> Log.info("%s  (%s)", Emoji.sparkles);

  if (Sys.file_exists(build_dir)) {
    Util.clean_directory(build_dir);
  };
};

let write = (path, (module_name, ast)) => {
  let {paths: {build_dir, source_dir}, module_type} = Config.get();
  let path = Printf.sprintf("%s.js", path) |> Filename.concat(build_dir);

  Filename.dirname(path) |> Core.Unix.mkdir_p;

  Config.root_path(path)
  |> Log.info("%s  %s (%s)", Emoji.printer, module_name);

  let out_channel = open_out(path);
  let write_out = output_string(out_channel);

  write_out("module.exports=");
  Generator.generate(
    write_out,
    {
      to_module_name: Util.normalize_module(source_dir),
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
