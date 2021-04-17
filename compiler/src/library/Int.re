/**
 Extension of the standard Int module with additional functionality.
 */
include Stdlib.Int;

let max_of = List.fold_left(max, 0);

let contains = ((min, max): (t, t), value: t) =>
  value >= min && value <= max;
