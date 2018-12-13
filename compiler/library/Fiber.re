let return = (x, stream) => Some((x, stream));
let mzero = _ => None;

let any =
  fun
  | LazyStream.Cons(token, input') => Some((token, Lazy.force(input')))
  | LazyStream.Nil => None;

let eof = x =>
  fun
  | LazyStream.Nil => Some((x, LazyStream.Nil))
  | _ => None;

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