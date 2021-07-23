/**
 Extension of the standard Hashtbl module with additional functionality.
 */
include Stdlib.Hashtbl;

let compare = (~compare=(==), l, r) =>
  length(l) == length(r)
  && to_seq_keys(l)
  |> List.of_seq
  |> List.for_all(key =>
       mem(r, key) && compare(find(l, key), find(r, key))
     );

let to_string =
    (
      key_to_string: 'a => string,
      value_to_string: 'b => string,
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
             find(tbl, key) |> value_to_string |> string,
           ]
           |> newline
         )
      |> concat
      |> indent(2),
      string("}"),
    ]
    |> concat
  );
