open Infix;

/**
 Extension of the standard Hashtbl module with additional functionality.
 */
include Stdlib.Hashtbl;

/**
 compare two Hashtbls by direct equality, or by using a custom [compare] function
 */
let compare = (~compare=(==), l, r) =>
  length(l) == length(r)
  && to_seq_keys(l)
  |> List.of_seq
  |> List.for_all(key =>
       mem(r, key) && compare(find(l, key), find(r, key))
     );

/**
 map the entries of a [tbl]
 */
let map = (map: (('a, 'b)) => ('c, 'd), tbl: t('a, 'b)): t('c, 'd) =>
  tbl |> to_seq |> Seq.map(map) |> of_seq;

/**
 map the keys of a [tbl]
 */
let map_keys = (map_key: 'a => 'c, tbl: t('a, 'b)): t('c, 'b) =>
  tbl |> map(Tuple.map_fst2(map_key));

/**
 map the values of a [tbl]
 */
let map_values = (map_value: 'b => 'c, tbl: t('a, 'b)): t('a, 'c) =>
  tbl |> map(Tuple.map_snd2(map_value));

let print =
    (
      key_to_string: 'a => string,
      print_value: 'b => Pretty.t,
      tbl: t('a, 'b),
    ) =>
  Pretty.(
    [
      [string("{")] |> newline,
      tbl
      |> to_seq_keys
      |> List.of_seq
      |> List.map(key =>
           [
             key |> key_to_string |> string,
             string(": "),
             find(tbl, key) |> print_value,
           ]
           |> newline
         )
      |> concat
      |> indent(2),
      string("}"),
    ]
    |> concat
  );

let to_string =
    (
      key_to_string: 'a => string,
      value_to_string: 'b => string,
      tbl: t('a, 'b),
    )
    : string =>
  tbl
  |> print(key_to_string, value_to_string % Pretty.string)
  |> Pretty.to_string;
