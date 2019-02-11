include Knot.Core;
include Knot.Fiber;

module M = Matchers;

let (|=) = (x, default) => opt(default, x);
