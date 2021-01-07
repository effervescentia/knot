/**
 * An abstraction around a source code module.
 * Includes information about its original path and import identifier.
 * May also allow for module data to be passed directly from an alternate source
 * such as when operating as a Language Server for an IDE.
 */
open Kore;
open File;

type path_t = {
  full: string,
  relative: string,
};

type t =
  | Raw(string)
  | File(path_t);

let create = () => {};

let of_string = (raw: string) => Raw(raw);

let of_file = (path: path_t) => File(path);

let read = f =>
  fun
  | File(path) => {
      let (stream, close) = IO.read_stream(path.full);

      (
        try(f(stream)) {
        | _ => []
        }
      )
      |> (
        r => {
          close();
          r;
        }
      );
    }
  | Raw(s) => CharStream.of_string(s) |> LazyStream.of_stream |> f;

let cache = cache =>
  fun
  | File(path) =>
    IO.clone(path.full, cache |> Cache.resolve_path(path.relative))
  | _ => ();
