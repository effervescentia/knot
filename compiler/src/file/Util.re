open Kore;

let __root_dir = "knot";

/**
 get the name of a unique folder within the temporary file directory
 */
let create_temp_dir = (sub_dir: string): string =>
  [__root_dir, sub_dir, Sys.time() |> string_of_float]
  |> List.fold_left(Filename.concat, Filename.get_temp_dir_name());
