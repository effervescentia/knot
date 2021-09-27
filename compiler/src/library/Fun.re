/**
 Functional programming utilities.
 */
include Stdlib.Fun;

/**
 create a generator from a list of values
 */
let generator = (values: list('a)) => {
  let remaining = ref(values);

  () => {
    switch (remaining^) {
    | [] => None
    | [x, ...xs] =>
      remaining := xs;
      Some(x);
    };
  };
};
