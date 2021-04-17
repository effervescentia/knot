open Infix;

type range_t = (Cursor.point_t, Cursor.point_t);

type entry_t('a) = (range_t, 'a);
type t('a) = BinaryTree.t(entry_t('a));

let is_in_range = ((start, end_): range_t, point: Cursor.point_t) =>
  Int.contains((start.line, end_.line), point.line)
  && (
    if (start.line == end_.line) {
      Int.contains((start.column, end_.column), point.column);
    } else if (start.line == point.line) {
      point.column >= start.column;
    } else if (end_.line == point.line) {
      point.column <= end_.column;
    } else {
      true;
    }
  );

let find_leaf = (point: Cursor.point_t, tree: t('a)): option(entry_t('a)) =>
  tree
  |> BinaryTree.search(
       ({value: (l_range, _)} as left, {value: (r_range, _)} as right) =>
       if (is_in_range(l_range, point)) {
         Some(left);
       } else if (is_in_range(r_range, point)) {
         Some(right);
       } else {
         None;
       }
     );
