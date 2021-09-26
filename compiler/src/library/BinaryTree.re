open Infix;

type t('a) = {
  value: 'a,
  left: option(t('a)),
  right: option(t('a)),
};

/* static */

let create = (~left=?, ~right=?, value: 'a) => {value, left, right};

/* methods */

let rec search = (choose: (t('a), t('a)) => option(t('a)), tree: t('a)) =>
  switch (tree) {
  | {value, left: Some(left), right: Some(right)} =>
    switch (choose(left, right)) {
    | Some(result) => search(choose, result)
    | None => Some(value)
    }
  | {value, left: Some(left)} as right
  | {value, right: Some(right)} as left =>
    switch (choose(left, right)) {
    | None => Some(value)
    | Some(result) when result == tree => Some(value)
    | Some(result) => search(choose, result)
    }
  | {value} => Some(value)
  };

let rec to_graph = (tree: t('a)): Graph.t('a) =>
  [tree.left, tree.right]
  |> List.fold_left(
       acc =>
         fun
         | Some(child) => {
             let graph = child |> to_graph |> Graph.union(acc);

             graph |> Graph.add_edge(tree.value, child.value);

             graph;
           }
         | None => acc,
       Graph.create([tree.value], []),
     );

/* pretty printing */

let pp = (pp_node: Fmt.t('a)): Fmt.t(t('a)) =>
  (ppf, tree: t('a)) => tree |> to_graph |> Graph.pp(pp_node, ppf);
