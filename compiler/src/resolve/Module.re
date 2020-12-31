/**
 * An abstraction around a source code module.
 * Includes information about its original path and import identifier.
 * May also allow for module data to be passed directly from an alternate source
 * such as when operating as a Language Server for an IDE.
 */
open Kore;

type t =
  | Raw(string)
  | File(string);

let create = () => {};

let of_string = (raw: string) => Raw(raw);

let of_file = (path: string) => File(path);

/* let read =
   fun
   | Raw(s) => CharStream.of_string(s)
   | File(path) => File.Reader.read_stream(path); */
