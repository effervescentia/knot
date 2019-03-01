open Kore;

let write = (config, path, ast) => {
  let path =
    Printf.sprintf("%s.js", path) |> Filename.concat(config.build_dir);

  Filename.dirname(path) |> Core.Unix.mkdir_p;

  Printf.sprintf("writing to %s", path) |> print_endline;

  let out_channel = open_out(path);
  Generator.generate(output_string(out_channel), abandon_ctx(ast));

  close_out(out_channel);
};
