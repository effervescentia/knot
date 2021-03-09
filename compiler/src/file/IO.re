/**
 Utitilies to read files in as a compatible module format.
 */
open Kore;

/**
 read a file as a lazy stream of unicode characters
 */
let read_stream = (path: string): (LazyStream.t(Char.t), unit => unit) =>
  open_in(path)
  |> (
    channel => (
      CharStream.of_channel(channel) |> LazyStream.of_stream,
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
