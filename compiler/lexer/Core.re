open Match;
open Matcher;

include Knot.Globals;
include Knot.Token;

/** tokens must be a contiguous set of characters */
let rec token = (~err=None, s, t) =>
  switch (String.length(s)) {
  | 0 => null(t)
  | 1 => Matcher(Exactly(s.[0]), t, err)
  | len =>
    let substring = String.sub(s, 1, len - 1);

    LookaheadMatcher(
      Exactly(s.[0]),
      Knot.Util.split(substring) |> List.map(c => Exactly(c)),
      _ => token(~err, substring, t) |> single,
      err,
    );
  };

/** glyphs can contain whitespace characters */;
let glyph = (s, t) => {
  let rec match_contd = (ss, _) =>
    [
      if (String.length(ss) == 1) {
        Matcher(Exactly(ss.[0]), t, None);
      } else {
        let substring = String.sub(ss, 1, String.length(ss) - 1);

        PermissiveLookaheadMatcher(
          Exactly(ss.[0]),
          whitespace,
          Knot.Util.split(substring) |> List.map(match => Exactly(match)),
          substring |> match_contd,
        );
      },
      LookaheadMatcher(
        whitespace,
        [Any([Exactly(ss.[0]), whitespace])],
        match_contd(ss),
        None,
      ),
    ]
    |> many;

  switch (String.length(s)) {
  | 0 => null(t)
  | 1 => Matcher(Exactly(s.[0]), t, None)
  | len =>
    let substring = String.sub(s, 1, len - 1);

    PermissiveLookaheadMatcher(
      Exactly(s.[0]),
      whitespace,
      Knot.Util.split(substring) |> List.map(match => Exactly(match)),
      substring |> match_contd,
    );
  };
};

let rec (===>) = (s, t) => token(s, _ => result(t));
