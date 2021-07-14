/**
 wrapper for working with a stream that is lazily evaluated

 copied from this original implementation in the library "opal"
 {v https://github.com/pyrocat101/opal/blob/ac495a4fc141cf843da74d223baecca47324acd4/opal.ml}
 */
type t('a) =
  | Cons('a, Lazy.t(t('a)))
  | Nil;

/* static */

let of_stream = (stream: Stream.t('a)): t('a) => {
  let rec next = stream =>
    try(Cons(Stream.next(stream), lazy(next(stream)))) {
    | Stream.Failure => Nil
    };

  next(stream);
};

let of_function = (f: unit => option('a)): t('a) => {
  let rec next = f =>
    switch (f()) {
    | Some(x) => Cons(x, lazy(next(f)))
    | None => Nil
    };

  next(f);
};

let of_string = (str: string): t(char) =>
  str |> Stream.of_string |> of_stream;

let of_channel = (ic: in_channel): t(char) =>
  ic |> Stream.of_channel |> of_stream;
