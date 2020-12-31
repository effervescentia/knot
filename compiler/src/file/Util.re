open Kore;

let __root_dir = "knot";

let create_temp_dir = (sub_dir, prefix) =>
  [__root_dir, sub_dir, Printf.sprintf("%s_%d", prefix, Date.now())]
  |> List.fold_left(Filename.concat, Filename.get_temp_dir_name());

let remove_dir = dir => FileUtil.rm(~recurse=true, [dir]);
