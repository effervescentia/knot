include Stdlib.List;

let incl = (x: 'a, xs: list('a)) =>
  if (mem(x, xs)) {
    xs;
  } else {
    [x, ...xs];
  };

let excl = (x: 'a, xs: list('a)) =>
  if (mem(x, xs)) {
    xs |> filter((!=)(x));
  } else {
    xs;
  };

let uniq_by = (compare: ('a, 'a) => bool) =>
  fold_left((acc, x) => exists(compare(x), acc) ? acc : [x, ...acc], []);

let compare_members = (l: list('a), r: list('a)) =>
  length(l) == length(r) && l |> for_all(x => mem(x, r));
