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
 read a string as a lazy stream of unicode characters
 */
let read_string = (contents: string): LazyStream.t(Input.t) =>
  contents |> InputStream.of_string |> LazyStream.of_stream;

/**
 copy an existing file to a new location
 */
let clone = (source: string, target: string) => {
  Filename.dirname(target) |> FileUtil.mkdir(~parent=true);
  FileUtil.cp([source], target);
};

/**
 read a file to a string, applying newline normalization
 */
let read_to_string = (path: string): string => {
  let channel = open_in(path);
  let decoder = decoder(`Channel(channel));

  let rec loop = buffer =>
    switch (Uutf.decode(decoder)) {
    | `Uchar(uchar) =>
      uchar |> Buffer.add_utf_8_uchar(buffer);
      loop(buffer);
    | `Malformed(_) =>
      Uutf.u_rep |> Buffer.add_utf_8_uchar(buffer);
      loop(buffer);
    | `End =>
      close_in(channel);
      Buffer.contents(buffer);
    | `Await => assert(false)
    };

  Buffer.create(in_channel_length(channel)) |> loop;
};

/**
 * delete and recreate a directory
 */
let purge = (dir: string) => {
  [dir] |> FileUtil.rm(~recurse=true);
  dir |> FileUtil.mkdir(~parent=true);
};
