let (%) = (f, g, x) => g(f(x));

let (@?) = (l, r) =>
  switch (l, r) {
  | (None, None) => None
  | (Some(l), Some(r)) => Some(l @ r)
  | (Some(_) as l, _) => l
  | (_, Some(_) as r) => r
  };
