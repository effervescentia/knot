open Core;

let add = M.plus >> return((x, y) => AddExpr(no_ctx(x), no_ctx(y)))
and sub = M.minus >> return((x, y) => SubExpr(no_ctx(x), no_ctx(y)))
and mul = M.asterisk >> return((x, y) => MulExpr(no_ctx(x), no_ctx(y)))
and div =
  M.forward_slash >> return((x, y) => DivExpr(no_ctx(x), no_ctx(y)))
and lt = M.l_chev >> return((x, y) => LTExpr(no_ctx(x), no_ctx(y)))
and lte =
  M.less_than_or_equal >> return((x, y) => LTEExpr(no_ctx(x), no_ctx(y)))
and gt = M.r_chev >> return((x, y) => GTExpr(no_ctx(x), no_ctx(y)))
and gte =
  M.greater_than_or_equal
  >> return((x, y) => GTEExpr(no_ctx(x), no_ctx(y)))
and or_ = M.logical_or >> return((x, y) => OrExpr(no_ctx(x), no_ctx(y)))
and and_ = M.logical_and >> return((x, y) => AndExpr(no_ctx(x), no_ctx(y)))
and equals = M.equals >> return((x, y) => EqualsExpr(no_ctx(x), no_ctx(y)))
and not_equals =
  M.not_equals
  >> return((x, y) =>
       NegatedExpr(no_ctx(EqualsExpr(no_ctx(x), no_ctx(y))))
     )
and ternary_ =
  M.question_mark >> return((x, y) => AndExpr(no_ctx(x), no_ctx(y)))
and dot =
  M.period
  >> return(x =>
       fun
       | Variable(y) => DotAccess(no_ctx(x), y)

       | _ => throw(InvalidDotAccess)
     );
