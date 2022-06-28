/**
 Extension of the standard List module with additional functionality.
 */
include Stdlib.List;

module TList = Tablecloth.List;

exception NoListMembers;

/**
 check if a list is empty
 */
let is_empty = (xs: list('a)): bool => length(xs) |> (==)(0);

/**
 return a list containing the items from [xs]
 including [x] if it was not already present
 */
let incl = (x: 'a, xs: list('a)): list('a) =>
  if (mem(x, xs)) {
    xs;
  } else {
    [x, ...xs];
  };

/**
 return a list containing the items from [xs]
 with all instances of [x] removed
 */
let excl = (x: 'a, xs: list('a)): list('a) =>
  if (mem(x, xs)) {
    xs |> filter((!=)(x));
  } else {
    xs;
  };

/**
 include all items [ys] into a list [xs] if not already present
 */
let incl_all = (xs: list('a), ys: list('a)): list('a) =>
  fold_left((acc, y) => incl(y, acc), xs, ys);

/**
 exclude all items [ys] from a list [xs]
 */
let excl_all = (xs: list('a), ys: list('a)): list('a) =>
  filter(y => !mem(y, xs), ys);

/**
 use [compare] to filter out duplicate values from a list
 */
let uniq_by = (compare: ('a, 'a) => bool) =>
  fold_left((acc, x) => exists(compare(x), acc) ? acc : [x, ...acc], []);

/**
 check if both lists contain the same members
 */
let compare_members = (ls: list('a), rs: list('a)): bool =>
  ls |> for_all(x => mem(x, rs)) && rs |> for_all(x => mem(x, ls));

/**
 get the first and last members of the list
 raises [NoListMembers] if the list is empty
 */
let ends = (xs: list('a)): ('a, 'a) =>
  if (is_empty(xs)) {
    raise(NoListMembers);
  } else {
    (nth(xs, 0), length(xs) - 1 |> nth(xs));
  };

/**
 generate a list by repeating a [value] a number of times
 */
let repeat = (count: int, value: 'a): list('a) =>
  TList.repeat(~count, value);

/**
 optionally get the last item of a list [xs]
 */
let last = (xs: list('a)): option('a) =>
  switch (xs) {
  | [] => None
  | _ => nth_opt(xs, length(xs) - 1)
  };

let intersperse = TList.intersperse;

let partition =
    (predicate: 'a => bool, xs: list('a)): (list('a), list('a)) =>
  TList.partition(~f=predicate, xs);

let split_at = (index: int, xs: list('a)) => TList.split_at(~index, xs);

/**
 split a list [xs] into two equal-ish sized lists
 */
let divide = (xs: list('a)) =>
  switch (xs) {
  | []
  | [_] => (xs, [])
  | _ => xs |> split_at(length(xs) / 2)
  };

/**
 returns [true] if the list is empty
 */
let is_empty = (xs: list('a)): bool => length(xs) == 0;
