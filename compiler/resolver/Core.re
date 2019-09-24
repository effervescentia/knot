include Knot.Core;
include Context;

module NestedHashtbl = Knot.NestedHashtbl;

let (=<<) = (x, y) => {
  y(x);

  x;
};
