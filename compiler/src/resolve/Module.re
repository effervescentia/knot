/**
 An abstraction around a source code module.
 Includes information about its original path and import identifier.
 May also allow for module data to be passed directly from an alternate source
 such as when operating as a Language Server for an IDE.
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

/* static */

let of_string = (raw: string) => Raw(raw);

let of_file = (path: path_t) => File(path);

/* methods */

let get_path =
  fun
  | Raw(_) => None
  | File(path) => Some(path);

let exists =
  fun
  | Raw(_) => true
  | File(path) => Sys.file_exists(path.full);

let read = f =>
  fun
  | File(path) as x when !exists(x) => Error([FileNotFound(path.relative)])
  | File(path) =>
    IO.read_stream(path.full)
    |> (
      ((stream, close)) =>
        f(stream)
        |> (
          r => {
            close();
            Ok(r);
          }
        )
    )
  | Raw(s) => Ok(s |> IO.read_string |> f);

let read_to_string =
  fun
  | File(path) as x when !exists(x) => Error([FileNotFound(path.relative)])
  | File({full}) => Ok(IO.read_to_string(full))
  | Raw(s) => Ok(s);

let cache = (cache: Cache.t) =>
  fun
  | File(path) =>
    if (Sys.file_exists(path.full)) {
      let cached_path = cache |> Cache.resolve_path(path.relative);

      IO.clone(path.full, cached_path);

      Ok(cached_path);
    } else {
      Error([FileNotFound(path.relative)]);
    }
  | Raw(_) => raise(NotImplemented);

/* pretty printing */

let pp: Fmt.t(t) =
  ppf =>
    fun
    | Raw(s) => Fmt.pf(ppf, "raw: %s", s)
    | File({full, relative}) =>
      Fmt.pf(ppf, "full: %s, relative: %s", full, relative);
