/**
 * This file is largely copied from the OCaml parsing library Opal.
 * The original code can be found here: https://github.com/pyrocat101/opal
 */
open Knot.Globals;

let return = (x, stream) => Some((x, stream));
let mzero = _ => None;

let any =
  fun
  | LazyStream.Cons(res, input) => Some((res, Lazy.force(input)))
  | LazyStream.Nil => None;

let eof = (x, input) =>
  switch (input) {
  | LazyStream.Nil => Some((x, input))
  | _ => None
  };

let (>>=) = (x, y, input) =>
  switch (x(input)) {
  | Some((result, stream)) => y(result, stream)
  | None => None
  };

let (<|>) = (x, y, input) =>
  switch (x(input)) {
  | Some(_) as res => res
  | None => y(input)
  };

let (==>) = (x, y) => x >>= (r => y(r) |> return);
let (>>) = (x, y) => x >>= (_ => y);
let (<<) = (x, y) => x >>= (r => y >>= (_ => return(r)));
let (<~>) = (x, xs) => x >>= (r => xs >>= (rs => return([r, ...rs])));

let satisfy = test =>
  any
  >>= (
    res =>
      if (test(res)) {
        return(res);
      } else {
        mzero;
      }
  );

let exactly = x => satisfy(y => y == x);

let opt = (default, x) => x <|> return(default);
let optional = x => opt((), x >> return());
let between = (op, ed, x) => op >> x << ed;

let rec skip_many = x => opt((), x >>= (_ => skip_many(x)));

let rec many = x =>
  opt([], x >>= (r => many(x) >>= (rs => return([r, ...rs]))));

let sep_by1 = (x, sep) => x <~> many(sep >> x);
let sep_by = (x, sep) => sep_by1(x, sep) <|> return([]);

let one_of = l => satisfy(x => List.mem(x, l));