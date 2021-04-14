/**
 Custom binary operators.
 */

let (%) = (f, g, x) => g(f(x));

let (@?) = (l, r) =>
  switch (l, r) {
  | (None, None) => None
  | (Some(l), Some(r)) => Some(l @ r)
  | (Some(_) as l, _) => l
  | (_, Some(_) as r) => r
  };

let (|?:) = (x, y) =>
  switch (x) {
  | Some(x') => x'
  | None => y
  };

let (|!:) = (x, y) =>
  switch (x) {
  | Some(x') => x'
  | None => y()
  };

let (|?>) = (x, f) =>
  switch (x) {
  | Some(x') => Some(f(x'))
  | None => None
  };

let (|?<) = (x, f) =>
  switch (x) {
  | Some(x') => f(x')
  | None => None
  };
