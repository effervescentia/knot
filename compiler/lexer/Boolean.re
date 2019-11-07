open Core;

let (==>) = (s, t) => token(s, _ => Boolean(t) |> result);

let matchers = ["true" ==> true, "false" ==> false];
