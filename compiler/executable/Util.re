open Kore;

let normalize_path = (root_dir, file) =>
  if (Filename.is_relative(file)) {
    Filename.concat(root_dir, file);
  } else {
    file;
  };

let pipe = (buffer_size, in_channel, out_channel) => {
  let channel_length = in_channel_length(in_channel);

  let rec write = () => {
    let remaining = channel_length - pos_in(in_channel);
    let buf_length = min(buffer_size, remaining);
    let buf = Bytes.create(buf_length);

    let read = input(in_channel, buf, 0, buf_length);
    output(out_channel, buf, 0, read);

    if (remaining !== 0) {
      write();
    };
  };

  write();

  flush(out_channel);
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

      pipe(buffer_size, in_channel, tmp_channel);

      close_out(tmp_channel);
      close_in(in_channel);

      open_in(tmp_file);
    }
  );

let from_option = exn =>
  fun
  | Some(x) => x
  | None => raise(exn);

let is_source_module = path => String.length(path) != 0 && path.[0] == '.';

let is_within_dir = (parent_dir, path) =>
  String.sub(path, 0, String.length(parent_dir)) != parent_dir;

let chop_path_prefix = (prefix, path) => {
  let prefix_length = String.length(prefix);

  String.sub(
    path,
    prefix_length + 1,
    String.length(path) - prefix_length - 1,
  );
};

let to_path_segment = path =>
  Str.global_replace(
    Str.regexp("\\."),
    Filename.dir_sep,
    String.sub(path, 1, String.length(path) - 1),
  );

let rec clean_directory =
  fun
  | res when Sys.is_directory(res) => {
      Sys.readdir(res) |> Array.iter(Filename.concat(res) % clean_directory);

      Unix.rmdir(res);
    }
  | res => Unix.unlink(res);
