open Kore;

let clean_build_dir = () => {
  let {paths: {build_dir}} = Config.get();
  Config.root_path(build_dir) |> Log.info("%s  (%s)", Emoji.sparkles);

  if (Sys.file_exists(build_dir)) {
    Util.clean_directory(build_dir);
  };
};

let write = (path, (module_name, ast)) => {
  let {paths: {build_dir}} = Config.get();
  let path = Printf.sprintf("%s.js", path) |> Filename.concat(build_dir);

  Filename.dirname(path) |> Core.Unix.mkdir_p;

  Config.root_path(path)
  |> Log.info("%s  %s (%s)", Emoji.printer, module_name);

  let out_channel = open_out(path);
  let write_out = output_string(out_channel);

  write_out("module.exports=");
  Generator.generate(write_out, abandon_ctx(ast));

  close_out(out_channel);
};
