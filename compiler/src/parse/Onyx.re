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
 * if parser x succeeds, drop the result and try to match parser y
 */
let (>>) = (x, y) => x >>= (_ => y);

/**
 * discard_right
 *
 * if parser x succeeds, attempt to match parser y but return the result of x
 */
let (<<) = (x, y) => x >>= (r => r <$ y);

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
 * returns the current context
 */
let ctx =
  LazyStream.(
    fun
    | Cons(r, _) as input => Some((Char.context(r), input))
    | Nil => None
  );

/**
 * matches a single character
 */
let satisfy = (f: 'a => bool) =>
  any >>= (x => Char.value(x) |> (v => f(v) ? return(x) : none));

/**
 * matches a single character
 */
let char = (c: char) => satisfy((==)(Uchar.of_char(c)));

/**
 * attempts to match each parser from left to right until one succeeds
 */
let rec choice =
  fun
  | [] => none
  | [x, ...xs] => x <|> choice(xs);

/**
 * matches any of the provided characters
 */
let one_of = (cs: list(char)) =>
  cs |> List.map(Uchar.of_char) |> (ucs => satisfy(v => List.mem(v, ucs)));

/**
 * matches none of the provided characters
 */
let none_of = (cs: list(char)) =>
  cs |> List.map(Uchar.of_char) |> (ucs => satisfy(v => !List.mem(v, ucs)));

/**
 * matches a range of characters
 */
let range = (l: char, r: char) =>
  satisfy(v => Uchar.of_char(l) <= v && Uchar.of_char(r) >= v);

/**
 * returns a default value if x not matched
 */
let option = (default, x) => x <|> return(default);

/**
 * returns nothing whether or not x is matched
 */
let optional = x => x >> none |> option();

/**
 * matches a pattern multiple times and return an empty result
 */
let rec skip_many = x => x >>= (_ => skip_many(x)) |> option();

/**
 * matches a pattern n+1 times and return an empty result
 */
let skip_many1 = x => x >> skip_many(x);

/**
 * matches a pattern multiple times and return a list of results
 */
let rec many = x =>
  x >>= (r => many(x) >|= (rs => [r, ...rs])) |> option([]);

/**
 * matches a pattern n+1 times and return a list of results
 */
let many1 = x => x <~> many(x);

/* matchers */

module Matchers = {
  let space = one_of([' ', '\t', '\n']);
  let spaces = skip_many(space);

  let underscore = char('_');

  let digit = range('0', '9');

  let lower_alpha = range('a', 'z');
  let upper_alpha = range('A', 'Z');
  let alpha = lower_alpha <|> upper_alpha;

  let alpha_num = digit <|> alpha;

  let lexeme = x => spaces >> x;

  let terminated = x => x << (char(';') |> optional);

  /**
   * matches a sequence of characters
   */
  let token = (s: string) =>
    ctx
    >>= (
      start => {
        let rec loop =
          fun
          | [] => assert(false)
          | [c] =>
            char(c)
            >>= Char.context
            % (end_ => return((s, Cursor.range(start, end_))))
          | [c, ...cs] => char(c) >> loop(cs);

        loop(s |> String.to_seq |> List.of_seq);
      }
    )
    |> lexeme;

  /**
   * matches a sequence of alpha-numeric characters
   */
  let identifier =
    choice([alpha, underscore])
    <~> (choice([alpha_num, underscore]) |> many)
    >|= (
      rs => {
        let (start, end_) = List.ends(rs) |> Tuple.map2(Char.context);
        let name = rs |> List.map(Char.value) |> String.of_uchars;

        (name, Cursor.range(start, end_));
      }
    )
    |> lexeme;
};
