open Infix;

/**
 wrapper for working with a stream that is lazily evaluated

 copied from this original implementation in the library "opal"
 {v https://github.com/pyrocat101/opal/blob/ac495a4fc141cf843da74d223baecca47324acd4/opal.ml}
 */
type t('a) =
  | Cons('a, Lazy.t(t('a)))
  | Nil;

/* static */

/**
 construct a LazyStream of values from a standard Stream of values
 */
let of_stream = (stream: Stream.t('a)): t('a) => {
  let rec next = stream =>
    try(Cons(Stream.next(stream), lazy(next(stream)))) {
    | Stream.Failure => Nil
    };

  next(stream);
};

/**
 construct a LazyStream from a generator function
 */
let of_function = (f: unit => option('a)): t('a) => {
  let rec next = f =>
    switch (f()) {
    | Some(x) => Cons(x, lazy(next(f)))
    | None => Nil
    };

  next(f);
};

/**
 construct a LazyStream of characters from a string
 */
let of_string = (str: string): t(char) =>
  str |> Stream.of_string |> of_stream;

/**
 construct a LazyStream of characters from an input channel
 */
let of_channel = (ic: in_channel): t(char) =>
  ic |> Stream.of_channel |> of_stream;

/* methods */

/**
 resolve all values in the stream into a list
 */
let to_list = (stream: t('a)): list('a) => {
  let rec loop =
    fun
    | Cons(x, next) => [x, ...loop(Lazy.force(next))]
    | Nil => [];

  loop(stream);
};

/* pretty printing */

let pp = (pp_value: Fmt.t('a)): Fmt.t(t('a)) =>
  ppf => to_list % Fmt.list(~sep=Fmt.nop, pp_value, ppf);
