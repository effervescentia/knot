open Infix;

type entry_t('a) = (Range.t, 'a);
type t('a) = BinaryTree.t(entry_t('a));

let find_leaf = (point: Point.t, tree: t('a)): option(entry_t('a)) => {
  let contains = Range.contains_point(point);

  tree
  |> BinaryTree.search(
       ({value: (l_range, _)} as left, {value: (r_range, _)} as right) =>
       if (contains(l_range)) {
         Some(left);
       } else if (contains(r_range)) {
         Some(right);
       } else {
         None;
       }
     );
};
