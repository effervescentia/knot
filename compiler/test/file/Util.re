open Kore;

let temp_file_name = (name, ext) =>
  Print.fmt("%s_%f.%s", name, Sys.time(), ext);

let get_temp_dir = () =>
  Filename.concat(
    Filename.get_temp_dir_name(),
    Sys.time() |> string_of_float,
  );

let write_to_file = (path, s) => {
  let out = open_out(path);
  Print.fprintf(out, "%s", s);

  close_out(out);
};

let append_to_file = (path, s) => {
  let out = open_out_gen([Open_append], 0o666, path);
  Print.fprintf(out, "%s", s);

  close_out(out);
};

let read_channel_to_string = in_ => {
  let string = really_input_string(in_, in_channel_length(in_));
  close_in(in_);

  string;
};

let read_file_to_string = path => {
  let in_ = open_in(path);
  let string = read_channel_to_string(in_);

  close_in(in_);

  string;
};

let read_lazy_char_stream = stream => {
  let buffer = Buffer.create(1);

  let rec loop = s => {
    switch (s) {
    | LazyStream.Cons((c, _), next) =>
      Buffer.add_utf_8_uchar(buffer, c);
      Lazy.force(next) |> loop;
    | LazyStream.Nil => Buffer.contents(buffer)
    };
  };

  loop(stream);
};
