module LazyStream = Opal.LazyStream;

let (%) = (f, g, x) => f(x) |> g;