include Knot.Globals;
include Knot.Fiber;
include Knot.Token;
include AST;

module M = Matchers;

let (|=) = (x, default) => opt(default, x);