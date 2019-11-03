open Core;

let rec matchers = [
  LookaheadMatcher(
    Numeric,
    Except([Numeric]),
    s => result(Number(int_of_string(s))),
  ),
  LookaheadMatcher(Numeric, Numeric, _ => matcher_list(matchers)),
];
