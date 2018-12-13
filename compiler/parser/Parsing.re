module TokenStream = KnotLex.TokenStream;

let return = (x, stream) => Some((x, stream));
let mzero = _ => None;

let any =
  fun
  | Some((res, input)) => Some((res, TokenStream.next(input)))
  | None => None;

let eof = (x, input) =>
  switch (TokenStream.next(input)) {
  | None => Some((x, input))
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

let rec many = x =>
  opt([], x >>= (r => many(x) >>= (rs => return([r, ...rs]))));