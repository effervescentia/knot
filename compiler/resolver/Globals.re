include Knot.Core;
include Context;

module NestedHashtbl = Knot.NestedHashtbl;

/** execute side effect with value */
let (=<<) = (x, y) => {
  y(x);

  x;
};
