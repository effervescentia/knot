open Core;

exception InvalidModule;

let pipe = (buffer_size, in_channel, out_channel) => {
  let channel_length = in_channel_length(in_channel);

  let rec write = () => {
    let remaining = channel_length - pos_in(in_channel);
    let buf_length = min(buffer_size, remaining);
    let buf = Bytes.create(buf_length);

    let read = input(in_channel, buf, 0, buf_length);
    output(out_channel, buf, 0, read);

    if (remaining != 0) {
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

      tmp_file;
    }
  );

let is_source_module = path =>
  String.length(path) > 1 && path.[0] == '.' && path.[1] != '/';

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
