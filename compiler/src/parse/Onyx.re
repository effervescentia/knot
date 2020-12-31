/**
 * based off of the libraries "opal" and "reparse"
 * https://github.com/pyrocat101/opal
 * https://github.com/lemaetech/reparse
 */
open Kore;

/* primitives */

/**
 * no match
 */
let none = _ => None;

/**
 * return a value
 */
let return = (x, input) => Some((x, input));

/**
 * match any input
 */
let any =
  LazyStream.(
    fun
    | Cons(token, input) => Some((token, Lazy.force(input)))
    | _ => None
  );

/**
 * match the end of the stream source
 */
let eof = x =>
  LazyStream.(
    fun
    | Nil => Some((x, Nil))
    | _ => None
  );

/* combinators */

/**
 * bind
 *
 * if parser x succeeds, returns a parser defined by calling f with the result of x
 *
 */
let (>>=) = (x, f, input) =>
  switch (x(input)) {
  | Some((result, input)) => f(result, input)
  | _ => None
  };

/**
 * map_result
 *
 * if parser x succeeds, transform the result with f
 */
let (>|=) = (x, f) => x >>= (r => f(r) |> return);

/**
 * apply
 *
 * if parser x succeeds, use the function it returns to map the result of parser y
 */
let (<*>) = (x, y) => x >>= (f => y >|= f);

/**
 * map
 *
 * wrap a function to apply to x
 */
let (<$>) = (f, x) => return(f) <*> x;

let map = (<$>);
let map2 = (f, x, y) => f <$> x <*> y;
let map3 = (f, x, y, z) => f <$> x <*> y <*> z;
let map4 = (f, w, x, y, z) => f <$> w <*> x <*> y <*> z;

/**
 * replace
 *
 * use v as the value when applied to x
 */
let (<$) = (v, x) => (_ => v) <$> x;

/**
 * discard_left
 *
 * if parser x succeeds, return parser y
 */
let ( *> ) = (x, y) => x >>= (_ => y);

/**
 * discard_right
 *
 * if parser x succeeds, attempt to match parser y but return the result of x
 */
let ( <* ) = (x, y) => x >>= (r => r <$ y);

/**
 * else
 *
 * if parser x succeeds, return the result, otherwise attempt to use y
 */
let (<|>) = (x, y, input) =>
  switch (x(input)) {
  | Some(_) as r => r
  | None => y(input)
  };

/**
 * finally
 *
 * if parser x succeeds, return the result, otherwise throw e
 */
let (<?>) = (x, e, input) =>
  switch (x(input)) {
  | Some(_) as r => r
  | None => raise(e)
  };

/**
 * chain
 *
 * if parser x succeeds, append it to the results of parser xs
 */
let (<~>) = (x, xs) => x >>= (r => xs >>= (rs => return([r, ...rs])));

/* helpers */

/**
 * attempts to match each parser from left to right until one succeeds
 */
let rec choice =
  fun
  | [] => none
  | [x, ...xs] => x <|> choice(xs);
