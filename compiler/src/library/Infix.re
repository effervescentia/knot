/**
 Custom infix operators.
 */

let (%) = (f, g, x) => g(f(x));

/**
 concatenate two optional lists
 */
let (@?) = (l, r) =>
  switch (l, r) {
  | (None, None) => None
  | (Some(l), Some(r)) => Some(l @ r)
  | (Some(_) as l, _) => l
  | (_, Some(_) as r) => r
  };

/**
 unpack the option [x] or fallback to the result of calling [y]
 */
let (|!:) = (x, y) =>
  switch (x) {
  | Some(x') => x'
  | None => y()
  };

/**
 unpack the option [x] or fallback to [y]
 */
let (|?:) = (x, y) => Option.value(~default=y, x);

/**
 optionally map the value of [x] with [f]
 where [f] must return an option
 */
let (|?<) = Option.bind;

/**
 optionally map the value of [x] with [f]
 */
let (|?>) = (x, f) => Option.map(f, x);

/**
 creates a to_string method from a pretty-printer
 */
let (~@) = Pretty.Formatters.to_to_string;
