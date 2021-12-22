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

/**
 memoize a function
 */
let memo = (get_key: 'a => 'b, f: 'a => 'c): ('a => 'c) => {
  let past = ref([]);

  let rec loop = x => {
    let key = get_key(x);

    try(List.assoc(key, past^)) {
    | Not_found =>
      let value = f(x);

      past := [(key, value), ...past^];

      value;
    };
  };

  loop;
};
