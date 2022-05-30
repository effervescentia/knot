open Kore;

type t = {
  name: string,
  mutable children: list(t),
};

/* static */

let create = (): t => {name: "", children: []};

/* methods */

let rec _create_node = (name: string, rest: list(string)): t => {
  name,
  children:
    switch (rest) {
    | [part, ...rest'] => [_create_node(part, rest')]
    | _ => []
    },
};

let rec find = (path: list(string), tree: t): option(t) =>
  tree.children
  |> List.find_map(node =>
       switch (path) {
       | [part, ...path'] when node.name == part =>
         List.is_empty(path') ? Some(node) : find(path', node)
       | _ => None
       }
     );

let exists = (path: list(string), tree: t): bool =>
  tree |> find(path) |> Option.is_some;

let is_leaf = (path: list(string), tree: t): bool =>
  tree |> find(path) |?> (node => List.is_empty(node.children)) |?: false;

let rec add = (path: list(string), tree: t): unit =>
  switch (path) {
  | [part, ...path'] =>
    let child = tree.children |> List.find_opt(node => node.name == part);

    switch (child) {
    /* node exists */
    | Some(_) when List.is_empty(path') => ()

    /* node is nested deeper */
    | Some(child') => child' |> add(path')

    /* node does not exist */
    | None => tree.children = [_create_node(part, path'), ...tree.children]
    };
  | _ => ()
  };

let rec remove = (path: list(string), tree: t): unit =>
  switch (path) {
  | [part, ...path'] =>
    let child = tree.children |> List.find_opt(node => node.name == part);

    switch (child) {
    /* node exists */
    | Some(child') when List.is_empty(path') =>
      tree.children = tree.children |> List.excl(child')

    /* node is nested deeper */
    | Some(child') => child' |> remove(path')

    /* node does not exist */
    | None => ()
    };
  | _ => ()
  };

let rec to_graph = (~parent=[], tree: t): Graph.t((list(string), string)) => {
  let graph = Graph.create([(parent, tree.name)], []);
  let path = parent @ [tree.name];

  tree.children
  |> List.fold_left(
       (acc, child) => {
         let res = to_graph(~parent=path, child) |> Graph.union(acc);

         res |> Graph.add_edge((parent, tree.name), (path, child.name));

         res;
       },
       graph,
     );
};

let pp: Fmt.t(t) =
  ppf =>
    to_graph % Graph.pp((ppf, (_, path)) => Fmt.string(ppf, path), ppf);