/**
 Extension of the standard String module with additional functionality.
 */
open Infix;

include Stdlib.String;

module TString = Tablecloth.String;

/* static */

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

let drop_prefix = (prefix: string, value: string): string =>
  starts_with(prefix, value) ? drop_left(length(prefix), value) : value;

let drop_suffix = (suffix: string, value: string): string =>
  ends_with(suffix, value) ? drop_right(length(suffix), value) : value;

let rec drop_all_prefix = (prefix: string, value: string): string =>
  starts_with(prefix, value)
    ? drop_left(length(prefix), value) |> drop_all_prefix(prefix) : value;

let rec drop_all_suffix = (suffix: string, value: string): string =>
  ends_with(suffix, value)
    ? drop_right(length(suffix), value) |> drop_all_suffix(suffix) : value;

let repeat = (count: int, value: string) => TString.repeat(~count, value);

let find_index = (pattern: string, value: string) =>
  switch (pattern) {
  | "" => Some(0)
  | _ =>
    let len = length(value);
    let rec loop = index =>
      index > len
        ? None
        : sub(value, index, len - index) |> starts_with(pattern)
            ? Some(index) : loop(index + 1);

    loop(0);
  };

let replace = (target: char, replacement: char) =>
  to_seq % Seq.map(x => x == target ? replacement : x) % of_seq;

let split = (pattern: string, value: string) =>
  switch (find_index(pattern, value)) {
  | Some(index) => (
      sub(value, 0, index),
      sub(value, index + 2, length(value) - (index + 2)),
    )

  | None => (value, "")
  };
