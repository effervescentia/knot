open Kore;

let __root_dir = "knot";

/**
 get the name of a unique folder within the temporary file directory
 */
let create_temp_dir = (sub_dir: string, prefix: string): string =>
  [__root_dir, sub_dir, Print.fmt("%s_%f", prefix, Sys.time())]
  |> List.fold_left(Filename.concat, Filename.get_temp_dir_name());

/**
 remove a directory and all of the files it contains
 */
let remove_dir = (dir: string) => FileUtil.rm(~recurse=true, [dir]);
