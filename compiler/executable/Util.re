open Core;

exception MissingRootDirectory;

let rec find_config_file = entry => {
  let dir = Filename.dirname(entry);
  let dir_handle = Unix.opendir(dir);

  let rec find = () =>
    switch (Unix.readdir(dir_handle)) {
    | name =>
      switch (String.lowercase_ascii(name)) {
      | ".knot.yml"
      | ".knot.yaml" => Some(Filename.concat(dir, name))
      | _ => find()
      }
    | exception End_of_file =>
      Unix.closedir(dir_handle);
      None;
    };

  find()
  |> (
    fun
    | Some(res) => res
    | None =>
      if (Filename.dirname(dir) == dir) {
        raise(MissingRootDirectory);
      } else {
        find_config_file(dir);
      }
  );
};

let normalize_path = (root_dir, file) =>
  if (Filename.is_relative(file)) {
    Filename.concat(root_dir, file);
  } else {
    file;
  };

let real_path = (root_dir, dep) =>
  if (dep.[0] == '.') {
    Str.split(Str.regexp_string("."), dep)
    |> List.fold_left((acc, x) => Filename.concat(acc, x), "")
    |> Filename.concat("src")
    |> Filename.concat(root_dir)
    |> Printf.sprintf("%s.kn");
  } else {
    Filename.concat("node_modules", dep) |> Filename.concat(root_dir);
  };
