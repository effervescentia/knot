/**
 Custom binary operators.
 */
open Extensions;

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
let (|?:) = (x, y) => x |!: (() => y);

/**
 optionally map the value of [x] with [f]
 where [f] must return an option
 */
let (|?<) = (x, f) =>
  switch (x) {
  | Some(x') => f(x')
  | None => None
  };

/**
 optionally map the value of [x] with [f]
 */
let (|?>) = (x, f) => x |?< (y => Some(f(y)));

/**
 creates a to_string method from a pretty-printer
 */
let (~@) = x => Fmt.root(x) |> Fmt.to_to_string;

/**
 creates a channel writer from a pretty-printer
 */
let (~$) = pp => Format.formatter_of_out_channel % pp;
