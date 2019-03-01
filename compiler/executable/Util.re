open Core;

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

let cache_as_tmp = (buffer_size, file) =>
  Filename.open_temp_file(
    ~mode=[Open_wronly, Open_binary],
    "knot-cached-module-",
    "-maybe-put-a-build-hash-here",
  )
  |> (
    ((tmp_file, tmp_channel)) => {
      let in_channel = open_in_bin(file);
      let channel_length = in_channel_length(in_channel);

      let rec write = () => {
        let remaining = channel_length - pos_in(in_channel);
        let buf_length = min(buffer_size, remaining);
        let buf = Bytes.create(buf_length);

        let read = input(in_channel, buf, 0, buf_length);
        output(tmp_channel, buf, 0, read);

        if (remaining !== 0) {
          write();
        };
      };

      write();

      flush(tmp_channel);

      close_out(tmp_channel);
      close_in(in_channel);

      open_in(tmp_file);
    }
  );
