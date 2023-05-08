open Knot.Kore;

module Token = {
  type t('prim) =
    | Skip
    | Join
    | Identifier(string)
    | Primitive('prim);
};

type t('prim) = RangeTree.t(Token.t('prim));

let skip: t('prim) = BinaryTree.create((Range.zero, Token.Skip));

let join = (left: t('prim), right: t('prim)) =>
  switch (left, right) {
  | ({value: (_, Skip), _}, {value: (_, Skip), _}) => skip
  | (only, {value: (_, Skip), _})
  | ({value: (_, Skip), _}, only) => only
  | _ =>
    BinaryTree.create(
      ~left,
      ~right,
      ((left.value |> fst |> fst, right.value |> fst |> snd), Join),
    )
  };

let fold = f => List.fold_left((acc, x) => join(acc, f(x)), skip);

let wrap = (range, tree: t('prim)) =>
  tree.value |> fst == range
    ? tree : BinaryTree.create(~left=tree, (range, Join));

/* static */

let of_untyped_id = ((id, (_, range))) =>
  BinaryTree.create((range, Token.Identifier(id)));

let rec of_list = (xs: list(t('prim))): t('prim) =>
  switch (xs) {
  | [] => skip
  | [x] => x
  | _ =>
    switch (xs |> List.divide |> Tuple.map2(of_list)) {
    | ({value: (_, Skip), _}, {value: (_, Skip), _}) => skip
    | ({value: (_, Skip), _}, only)
    | (only, {value: (_, Skip), _}) => only
    | (head, tail) => join(head, tail)
    }
  };
