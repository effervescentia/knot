open Infix;

type entry_t('a) = (Cursor.range_t, 'a);
type t('a) = BinaryTree.t(entry_t('a));

let find_leaf = (point: Cursor.point_t, tree: t('a)): option(entry_t('a)) =>
  tree
  |> BinaryTree.search(
       ({value: (l_range, _)} as left, {value: (r_range, _)} as right) =>
       if (Cursor.is_in_range(l_range, point)) {
         Some(left);
       } else if (Cursor.is_in_range(r_range, point)) {
         Some(right);
       } else {
         None;
       }
     );
