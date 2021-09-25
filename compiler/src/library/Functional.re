/**
 Functional programming utilities.
 */

/**
 returns any value passed to it
 */
let identity = x => x;

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
