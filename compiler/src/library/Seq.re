include Stdlib.Seq;

let append = (lhs: t('a), rhs: t('a)): t('a) =>
  (() => Cons(lhs, () => Cons(rhs, () => Nil))) |> flat_map(Fun.id);
