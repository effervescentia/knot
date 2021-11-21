open Infix;
open Extensions;

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

/* pretty printing */

let pp = (pp_key: Fmt.t('a), pp_value: Fmt.t('b)): Fmt.t(t('a, 'b)) =>
  Fmt.hashtbl(Fmt.attr(pp_key, pp_value));
