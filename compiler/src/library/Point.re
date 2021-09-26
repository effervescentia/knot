type t = (int, int);

/* static */

let create = (line: int, column: int): t => (line, column);

let zero = create(0, 0);

/* getters */

let get_line = ((line, _): t): int => line;
let get_column = ((_, column): t): int => column;

/* methods */

/**
 compare the lines and columns of two points
 returns an integer to indicate their relation to each other
 */
let compare = ((l_line, l_column): t, (r_line, r_column): t): int =>
  switch (Int.compare(l_line, r_line)) {
  | 0 => Int.compare(l_column, r_column)
  | x => x
  };

/**
 find the smaller of two points
 */
let min = (lhs: t, rhs: t): t => compare(lhs, rhs) <= 0 ? lhs : rhs;

/**
 find the larger of two points
 */
let max = (lhs: t, rhs: t): t => compare(lhs, rhs) >= 0 ? lhs : rhs;

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, (line, column): t) => Fmt.pf(ppf, "(%d:%d)", line, column);
