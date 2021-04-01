/**
 Utitilies to read files in as a compatible module format.
 */
open Kore;

/**
 read a file as a lazy stream of unicode characters
 */
let read_stream = (path: string): (LazyStream.t(Input.t), unit => unit) =>
  open_in(path)
  |> (
    channel => (
      InputStream.of_channel(channel) |> LazyStream.of_stream,
      () => close_in(channel),
    )
  );

/**
 copy an existing file to a new location
 */
let clone = (source: string, target: string) => {
  Filename.dirname(target) |> FileUtil.mkdir(~parent=true);
  FileUtil.cp([source], target);
};

let read_to_string = path => {
  let in_ = open_in(path);
  let string = really_input_string(in_, in_channel_length(in_));

  close_in(in_);

  string;
};
