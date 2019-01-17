open Core;

let add = input => (M.plus >> return((x, y) => AddExpr(x, y)))(input)
and sub = input => (M.minus >> return((x, y) => SubExpr(x, y)))(input)
and mul = input => (M.asterisk >> return((x, y) => MulExpr(x, y)))(input)
and div = input =>
  (M.forward_slash >> return((x, y) => DivExpr(x, y)))(input)
and lt = input => (M.l_chev >> return((x, y) => LTExpr(x, y)))(input)
and lte = input =>
  (M.less_than_or_equal >> return((x, y) => LTEExpr(x, y)))(input)
and gt = input => (M.r_chev >> return((x, y) => GTExpr(x, y)))(input)
and gte = input =>
  (M.greater_than_or_equal >> return((x, y) => GTEExpr(x, y)))(input)
and or_ = input => (M.logical_or >> return((x, y) => OrExpr(x, y)))(input)
and and_ = input =>
  (M.logical_and >> return((x, y) => AndExpr(x, y)))(input)
and dot = input => (M.period >> return((x, y) => DotAccess(x, y)))(input);
