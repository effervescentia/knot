open Core;

let add = M.plus >> return((x, y) => AddExpr(x, y))
and sub = M.minus >> return((x, y) => SubExpr(x, y))
and mul = M.asterisk >> return((x, y) => MulExpr(x, y))
and div = M.forward_slash >> return((x, y) => DivExpr(x, y))
and lt = M.l_chev >> return((x, y) => LTExpr(x, y))
and lte = M.less_than_or_equal >> return((x, y) => LTEExpr(x, y))
and gt = M.r_chev >> return((x, y) => GTExpr(x, y))
and gte = M.greater_than_or_equal >> return((x, y) => GTEExpr(x, y))
and or_ = M.logical_or >> return((x, y) => OrExpr(x, y))
and and_ = M.logical_and >> return((x, y) => AndExpr(x, y));