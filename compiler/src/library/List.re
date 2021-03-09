/**
 Extension of the standard List module with additional functionality.
 */
include Stdlib.List;

module TList = Tablecloth.List;

let is_empty = (xs: list('a)): bool => length(xs) |> (==)(0);

let incl = (x: 'a, xs: list('a)): list('a) =>
  if (mem(x, xs)) {
    xs;
  } else {
    [x, ...xs];
  };

let excl = (x: 'a, xs: list('a)): list('a) =>
  if (mem(x, xs)) {
    xs |> filter((!=)(x));
  } else {
    xs;
  };

let excl_all = (xs: list('a), ys: list('a)): list('a) =>
  filter(y => !mem(y, xs), ys);

let uniq_by = (compare: ('a, 'a) => bool) =>
  fold_left((acc, x) => exists(compare(x), acc) ? acc : [x, ...acc], []);

let compare_members = (l: list('a), r: list('a)): bool =>
  l |> for_all(x => mem(x, r));

let ends = (l: list('a)): ('a, 'a) => (
  nth(l, 0),
  length(l) - 1 |> nth(l),
);

let repeat = (count: int, value: 'a): list('a) =>
  TList.repeat(~count, value);

let last = (xs: list('a)): option('a) =>
  switch (xs) {
  | [] => None
  | _ => nth_opt(xs, length(xs) - 1)
  };
