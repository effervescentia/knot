open Infix;

type entry_t('a) = (Range.t, 'a);
type t('a) = BinaryTree.t(entry_t('a));

let find_leaf = (point: Point.t, tree: t('a)): option(entry_t('a)) =>
  tree
  |> BinaryTree.search(
       ({value: (l_range, _)} as left, {value: (r_range, _)} as right) =>
       if (Range.contains(l_range, point)) {
         Some(left);
       } else if (Range.contains(r_range, point)) {
         Some(right);
       } else {
         None;
       }
     );
