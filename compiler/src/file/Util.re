open Kore;

let __root_dir = "knot";

let create_temp_dir = (sub_dir: string, prefix: string): string =>
  [__root_dir, sub_dir, Print.fmt("%s_%f", prefix, Date.now())]
  |> List.fold_left(Filename.concat, Filename.get_temp_dir_name());

let remove_dir = (dir: string) => FileUtil.rm(~recurse=true, [dir]);
