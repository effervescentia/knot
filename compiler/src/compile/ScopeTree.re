open Kore;
open Reference;

type t = RangeTree.t(option(Hashtbl.t(Export.t, Type.t)));

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

let rec of_def_tbl = (~range=?, declarations: DeclarationTable.t): t => {
  declarations.children
  |> List.map(((x, range)) => of_def_tbl(~range, x))
  |> List.divide
  |> Tuple.map2(_join)
  |> (
    fun
    | (
        Some({value: ((start, _), _)} as head),
        Some({value: ((_, end_), _)} as tail),
      ) =>
      BinaryTree.create(
        ~left=head,
        ~right=tail,
        (range |?: (start, end_), Some(declarations.scope)),
      )
    | (Some({value: (only_range, _)} as only), None)
    | (None, Some({value: (only_range, _)} as only)) =>
      BinaryTree.create(
        ~left=only,
        (range |?: only_range, Some(declarations.scope)),
      )
    | (None, None) =>
      BinaryTree.create((range |?: Range.zero, Some(declarations.scope)))
  );
};

let rec of_context = (~range=?, context: ParseContext.t): t => {
  /* TODO: re-implement this with ParseContext */
  /* context.inner_modules */
  []
  |> List.map(((_, x, range)) => of_def_tbl(~range, x))
  |> _join
  |?: BinaryTree.create((range |?: Range.zero, Some(Hashtbl.create(0))));
};

let find_scope =
    (point: Point.t, tree: t): option(Hashtbl.t(Export.t, Type.t)) => {
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
  find_scope(point, tree) |?< (types => Hashtbl.find_opt(types, Named(id)));
