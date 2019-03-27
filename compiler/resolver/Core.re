include Knot.Core;
include Exception;
include Context;

module NestedHashtbl = Knot.NestedHashtbl;

let (=<<) = (x, y) => {
  y(x);

  x;
};
