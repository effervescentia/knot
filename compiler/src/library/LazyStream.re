/**
 * copied from this original implementation in the library "opal"
 * https://github.com/pyrocat101/opal/blob/ac495a4fc141cf843da74d223baecca47324acd4/opal.ml
 */

type t('a) =
  | Cons('a, Lazy.t(t('a)))
  | Nil;

let of_stream = stream => {
  let rec next = stream =>
    try(Cons(Stream.next(stream), lazy(next(stream)))) {
    | Stream.Failure => Nil
    };
  next(stream);
};

let of_function = f => {
  let rec next = f =>
    switch (f()) {
    | Some(x) => Cons(x, lazy(next(f)))
    | None => Nil
    };
  next(f);
};

let of_string = str => str |> Stream.of_string |> of_stream;

let of_channel = ic => ic |> Stream.of_channel |> of_stream;
