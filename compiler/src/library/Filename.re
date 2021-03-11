/**
 Extension of the standard Filename module with additional functionality.
 */
include Stdlib.Filename;

let __relative_prefix = current_dir_name ++ dir_sep;
let __dir_sep_char = List.nth(dir_sep |> String.to_list, 0);

let _drop_current_dirs =
  List.filter(value => value != "" && value != current_dir_name);
let _merge_parent_dirs = parts => {
  let navigate_up = ref(0);

  let parts =
    parts
    |> List.rev
    |> List.filter_map(
         fun
         | x when x == parent_dir_name => {
             navigate_up := navigate_up^ + 1;
             None;
           }
         | _ when navigate_up^ != 0 => {
             navigate_up := navigate_up^ - 1;
             None;
           }
         | x => Some(x),
       )
    |> List.rev;

  List.repeat(navigate_up^, parent_dir_name) @ parts;
};

/**
 simplify [path] to its most basic form
 */
let normalize = (path: string) => {
  let parts =
    String.split_on_char(__dir_sep_char, path) |> _drop_current_dirs;

  (String.starts_with(dir_sep, path) ? dir_sep : "")
  ++ (parts |> _merge_parent_dirs |> String.join(~separator=dir_sep));
};

/**
 concatenate two paths
 */
let rec concat = (l: string, r: string) =>
  switch (normalize(l), is_relative(r) ? normalize(r) : relative_to(l, r)) {
  | (l', "") => l'
  | (l', r') => Stdlib.Filename.concat(l', r') |> normalize
  }

/**
 find the path of [source] relative to [target]
 */
and relative_to = (target: string, source: string) => {
  let target_parts = resolve(target) |> String.split_on_char(__dir_sep_char);
  let source_parts = resolve(source) |> String.split_on_char(__dir_sep_char);

  let rec loop =
    fun
    | ([t, ...ts], [s, ...ss]) when t == s => loop((ts, ss))
    | (ts, ss) =>
      (ts |> List.map(_ => parent_dir_name))
      @ ss
      |> String.join(~separator=dir_sep);

  loop((target_parts, source_parts));
}

/**
 resolve the absolute form of [path]
 */
and resolve = (path: string) =>
  if (is_relative(path)) {
    concat(Sys.getcwd(), path);
  } else {
    normalize(path);
  };
