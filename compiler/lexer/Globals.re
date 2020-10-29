open Match;
open Matcher;

include Knot.Core;

let result = x => (Some(x), []);

let empty = (None, []);

let many = matchers => (None, matchers);

let single = matcher => many([matcher]);

let rec token = (s, t) =>
  switch (String.length(s)) {
  | 0 => invariant(InvalidToken)
  | len =>
    Matcher(
      Exactly(s.[0]),
      len == 1 ? t : (_ => [token(String.sub(s, 1, len - 1), t)] |> many),
    )
  };
