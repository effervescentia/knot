/**
 Extension of the standard String module with additional functionality.
 */
open Infix;

include Stdlib.String;

module TString = Tablecloth.String;

/* static */

/**
 construct a string from a list of Uchars
 */
let of_uchars = (cs: list(Uchar.t)) =>
  cs
  |> List.fold_left(
       (acc, r) => {
         Buffer.add_utf_8_uchar(acc, r);
         acc;
       },
       Buffer.create(List.length(cs)),
     )
  |> Buffer.contents;

/* methods */

let to_list = TString.to_list;

let join = (~separator="", xs: list(string)): string =>
  TString.join(~sep=separator, xs);

let starts_with = (prefix: string, value: string): bool =>
  TString.starts_with(~prefix, value);

let ends_with = (suffix: string, value: string): bool =>
  TString.ends_with(~suffix, value);

let drop_left = (count: int, value: string): string =>
  TString.drop_left(~count, value);

let drop_right = (count: int, value: string): string =>
  TString.drop_right(~count, value);

/**
 remove a [prefix] from the beginning of a string
 */
let drop_prefix = (prefix: string, value: string): string =>
  starts_with(prefix, value) ? drop_left(length(prefix), value) : value;

/**
 remove a [suffix] from the end of a string
 */
let drop_suffix = (suffix: string, value: string): string =>
  ends_with(suffix, value) ? drop_right(length(suffix), value) : value;

/**
 repeatedly remove a [prefix] from the beginning of a string
 until it no longer appears at the beginning of the resulting string
 */
let rec drop_all_prefix = (prefix: string, value: string): string =>
  starts_with(prefix, value)
    ? drop_left(length(prefix), value) |> drop_all_prefix(prefix) : value;

/**
 repeatedly remove a [suffix] from the end of a string
 until it no longer appears at the end of the resulting string
 */
let rec drop_all_suffix = (suffix: string, value: string): string =>
  ends_with(suffix, value)
    ? drop_right(length(suffix), value) |> drop_all_suffix(suffix) : value;

let repeat = (count: int, value: string) => TString.repeat(~count, value);

/**
 return the index of the first instance of a [pattern] within some [source]
 */
let find_index = (pattern: string, source: string) =>
  switch (pattern) {
  | "" => Some(0)
  | _ =>
    let len = length(source);
    let rec loop = index =>
      index > len
        ? None
        : sub(source, index, len - index) |> starts_with(pattern)
            ? Some(index) : loop(index + 1);

    loop(0);
  };

/**
 replace every instance of a character
 */
let replace = (target: char, replacement: char) =>
  to_seq % Seq.map(x => x == target ? replacement : x) % of_seq;

/**
 split a string around some [pattern]
 */
let split = (pattern: string, value: string) =>
  switch (find_index(pattern, value)) {
  | Some(0) => ("", value)

  | Some(index) => (
      sub(value, 0, index),
      sub(value, index + 2, length(value) - (index + 2)),
    )

  | None => (value, "")
  };
