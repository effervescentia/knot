include Stdlib.Hashtbl;

let compare = (~compare=(==), l, r) =>
  length(l) == length(r)
  && to_seq_keys(l)
  |> List.of_seq
  |> List.for_all(key =>
       mem(r, key) && compare(find(l, key), find(r, key))
     );
