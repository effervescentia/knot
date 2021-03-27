/**
 Recursive descent parsing utilities.

 based off of the libraries "opal" and "reparse"
 https://github.com/pyrocat101/opal
 https://github.com/lemaetech/reparse
 */
open Kore;

/* primitives */

/**
 no match
 */
let none = _ => None;

/**
 return a value
 */
let return = (x, input) => Some((x, input));

/**
 match any input
 */
let any =
  LazyStream.(
    fun
    | Cons(token, input) => Some((token, Lazy.force(input)))

    | _ => None
  );

/**
 match the end of the stream source
 */
let eof = x =>
  LazyStream.(
    fun
    | Nil => Some((x, Nil))
    | _ => None
  );

/* combinators */

/**
 {b bind}

 if parser [x] succeeds, returns a parser defined by calling [f] with the result of [x]
 */
let (>>=) = (x, f, input) =>
  switch (x(input)) {
  | Some((result, input)) => f(result, input)
  | _ => None
  };

/**
 {b map_result}

 if parser [x] succeeds, transform the result with [f]
 */
let (>|=) = (x, f) => x >>= (r => f(r) |> return);

/**
 {b produce_effect}

 if parser [x] succeeds, execute effect [f]
 */
let (>@=) = (x, f) =>
  x
  >>= (
    r => {
      f(r);
      return(r);
    }
  );

/**
 {b apply}

 if parser [x] succeeds, use the function it returns to map the result of parser [y]
 */
let (<*>) = (x, y) => x >>= (f => y >|= f);

/**
 {b map}

 wrap a function to apply to [x]
 */
let (<$>) = (f, x) => return(f) <*> x;

let map = (<$>);
let map2 = (f, x, y) => f <$> x <*> y;
let map3 = (f, x, y, z) => f <$> x <*> y <*> z;
let map4 = (f, w, x, y, z) => f <$> w <*> x <*> y <*> z;

/**
 {b replace}

 use [v] as the value when applied to [x]
 */
let (<$) = (v, x) => (_ => v) <$> x;

/**
 {b discard_left}

 if parser [x] succeeds, drop the result and try to match parser [y]
 */
let (>>) = (x, y) => x >>= (_ => y);

/**
 {b discard_right}

 if parser [x] succeeds, attempt to match parser [y] but return the result of [x]
 */
let (<<) = (x, y) => x >>= (r => r <$ y);

/**
 {b else}

 if parser [x] succeeds, return the result, otherwise attempt to use [y]
 */
let (<|>) = (x, y, input) =>
  switch (x(input)) {
  | Some(_) as r => r
  | None => y(input)
  };

/**
 {b finally}

 if parser [x] succeeds, return the result, otherwise throw [e]
 */
let (<?>) = (x, e, input) =>
  switch (x(input)) {
  | Some(_) as r => r
  | None => raise(e)
  };

/**
 {b chain}

 if parser [x] succeeds, append it to the results of parser [xs]
 */
let (<~>) = (x, xs) => x >>= (r => xs >|= (rs => [r, ...rs]));

/* helpers */

/**
 returns the current cursor
 */
let get_cursor =
  LazyStream.(
    fun
    | Cons(r, _) as input => Some((Input.cursor(r), input))
    | Nil => None
  );

/**
 matches a single character
 */
let satisfy = (f: 'a => bool) =>
  any >>= (x => Input.value(x) |> (v => f(v) ? return(x) : none));

/**
 matches a single character
 */
let char = (c: char) => satisfy((==)(Uchar.of_char(c)));

/**
 attempts to match each parser from left to right until one succeeds
 */
let rec choice =
  fun
  | [] => none
  | [x, ...xs] => x <|> choice(xs);

/**
 matches any of the provided characters
 */
let one_of = (cs: list(char)) =>
  cs |> List.map(Uchar.of_char) |> (ucs => satisfy(v => List.mem(v, ucs)));

/**
 matches none of the provided characters
 */
let none_of = (cs: list(char)) =>
  cs |> List.map(Uchar.of_char) |> (ucs => satisfy(v => !List.mem(v, ucs)));

/**
 matches a range of characters
 */
let range = (l: char, r: char) =>
  satisfy(v => Uchar.of_char(l) <= v && v <= Uchar.of_char(r));

/**
 returns value [default] if [x] not matched
 */
let option = (default, x) => x <|> return(default);

/**
 returns nothing whether or not [x] is matched
 */
let optional = x => option((), () <$ x);

/**
 matches a pattern [0] or more times and return an empty result
 */
let rec skip_many = x => x >>= (_ => skip_many(x)) |> option();

/**
 matches a pattern [n+1] times and return an empty result
 */
let skip_many1 = x => x >> skip_many(x);

/**
 matches a pattern [0] or more times and return a list of results
 */
let rec many = x =>
  x >>= (r => many(x) >|= (rs => [r, ...rs])) |> option([]);

/**
 matches a pattern [n+1] times and return a list of results
 */
let many1 = x => x <~> many(x);

/**
 matches a pattern [n+1] times divided by separator [sep]
 */
let sep_by1 = (sep, x) => x <~> many(sep >> x);

/**
 matches a pattern [0] or more times divided by separator [sep]
 */
let sep_by = (sep, x) => sep_by1(sep, x) <|> return([]);

/**
 matches a pattern [n+1] times separated by operator [op]

 associativity: {i left-to-right}
 */
let chainl1 = (x, op) => {
  let rec loop = a =>
    op >>= (f => x >>= (b => loop(f((a, b))))) <|> return(a);
  x >>= loop;
};

/**
 matches a pattern [0] or more times separated by operator [op]

 associativity: {i left-to-right}
 */
let chainl = (x, op, default) => chainl1(x, op) <|> return(default);

/**
 matches a pattern [n+1] times separated by operator [op]

 associativity: {i right-to-left}
 */
let rec chainr1 = (x, op) =>
  x >>= (a => op >>= (f => chainr1(x, op) >|= f(a)) <|> return(a));

/**
 matches a pattern [0] or more times separated by operator [op]

 associativity: {i right-to-left}
 */
let chainr = (x, op, default) => chainr1(x, op) <|> return(default);
