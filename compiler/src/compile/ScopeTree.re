open Kore;
open Reference;

type t = RangeTree.t(option(Hashtbl.t(Export.t, Type2.t)));

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

let rec of_def_tbl = (~cursor=?, definitions: DefinitionTable.t): t => {
  definitions.children
  |> List.map(((x, cursor)) => of_def_tbl(~cursor, x))
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
        (
          cursor |?> Cursor.expand |?: (start, end_),
          Some(definitions.scope),
        ),
      )
    | (Some({value: (only_cursor, _)} as only), None)
    | (None, Some({value: (only_cursor, _)} as only)) =>
      BinaryTree.create(
        ~left=only,
        (cursor |?> Cursor.expand |?: only_cursor, Some(definitions.scope)),
      )
    | (None, None) =>
      BinaryTree.create((
        cursor |?: Cursor.zero |> Cursor.expand,
        Some(definitions.scope),
      ))
  );
};

let rec of_context = (~cursor=?, context: NamespaceContext.t): t => {
  context.inner_modules
  |> List.map(((_, x, cursor)) => of_def_tbl(~cursor, x))
  |> _join
  |?: BinaryTree.create((
        cursor |?: Cursor.zero |> Cursor.expand,
        Some(Hashtbl.create(0)),
      ));
};

let find_scope =
    (point: Cursor.point_t, tree: t): option(Hashtbl.t(Export.t, Type2.t)) =>
  BinaryTree.search(
    (left, right) =>
      if (Cursor.is_in_range(left.value |> fst, point)) {
        Some(left);
      } else if (Cursor.is_in_range(right.value |> fst, point)) {
        Some(right);
      } else {
        None;
      },
    tree,
  )
  |?< snd;

let find_type =
    (id: Identifier.t, point: Cursor.point_t, tree: t): option(Type2.t) =>
  find_scope(point, tree) |?< (types => Hashtbl.find_opt(types, Named(id)));
