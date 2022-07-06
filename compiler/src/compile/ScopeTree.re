open Kore;
open Reference;

type t = RangeTree.t(option(list((Export.t, Type.t))));

let rec _join =
  fun
  | [] => None
  | [x] => Some(x)
  | xs =>
    xs
    |> List.divide
    |> Tuple.map2(_join)
    |> BinaryTree.(
         (
           fun
           | (
               Some({value: ((start, _), _)} as left),
               Some({value: ((_, end_), _)} as right),
             ) =>
             Some(create(~left, ~right, ((start, end_), None)))
           | (Some(_) as only, _)
           | (_, Some(_) as only) => only
           | _ => None
         )
       );

let rec of_context = (~range=?, context: ParseContext.t): t => {
  /* TODO: re-implement this with ParseContext */
  [] |> _join |?: BinaryTree.create((range |?: Range.zero, Some([])));
};

let find_scope =
    (point: Point.t, tree: t): option(list((Export.t, Type.t))) => {
  let contains = Range.contains_point(point);

  BinaryTree.search(
    (left, right) =>
      if (left.value |> fst |> contains) {
        Some(left);
      } else if (right.value |> fst |> contains) {
        Some(right);
      } else {
        None;
      },
    tree,
  )
  |?< snd;
};

let find_type = (id: string, point: Point.t, tree: t): option(Type.t) =>
  find_scope(point, tree) |?< List.assoc_opt(Export.Named(id));
