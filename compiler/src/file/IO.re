/**
 * Utitilies to read files in as a compatible module format.
 */
open Kore;

let read_stream = (path: string): (LazyStream.t(Char.t), unit => unit) =>
  open_in(path)
  |> (
    channel => (
      CharStream.of_channel(channel) |> LazyStream.of_stream,
      () => close_in(channel),
    )
  );

let clone = (source: string, target: string) => {
  Filename.dirname(target) |> FileUtil.mkdir(~parent=true);
  FileUtil.cp([source], target);
};
