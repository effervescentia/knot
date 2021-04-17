open Kore;
open Reference;

type t = RangeTree.t(option(Hashtbl.t(Export.t, Type.t)));

let rec of_context = (~cursor=?, context: Context.t): t => {
  let rec join =
    fun
    | [] => None
    | [x] => Some(x)
    | xs =>
      xs
      |> List.divide
      |> Tuple.map2(join)
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

  context.children
  |> List.map(((x, cursor)) => of_context(~cursor, x))
  |> List.divide
  |> Tuple.map2(join)
  |> (
    fun
    | (
        Some({value: ((start, _), _)} as head),
        Some({value: ((_, end_), _)} as tail),
      ) =>
      BinaryTree.create(
        ~left=head,
        ~right=tail,
        (
          cursor |?> Cursor.expand |?: (start, end_),
          Some(context.scope.types),
        ),
      )
    | (Some({value: (only_cursor, _)} as only), None)
    | (None, Some({value: (only_cursor, _)} as only)) =>
      BinaryTree.create(
        ~left=only,
        (
          cursor |?> Cursor.expand |?: only_cursor,
          Some(context.scope.types),
        ),
      )
    | (None, None) =>
      BinaryTree.create((
        cursor |?: Cursor.zero |> Cursor.expand,
        Some(context.scope.types),
      ))
  );
};

let find_type =
    (id: Identifier.t, point: Cursor.point_t, tree: t): option(Type.t) =>
  BinaryTree.search(
    (left, right) =>
      if (RangeTree.is_in_range(left.value |> fst, point)) {
        Some(left);
      } else if (RangeTree.is_in_range(right.value |> fst, point)) {
        Some(right);
      } else {
        None;
      },
    tree,
  )
  |?< (
    fun
    | (_, Some(types)) => Hashtbl.find_opt(types, Named(id))
    | _ => None
  );
