open Core;

let _first_char_pattern = Match.(Any([underscore, alphanumeric]));
let _subsequent_char_pattern = Match.(Any([underscore, alpha]));

let rec _match_subsequent_chars =
  Matcher.(
    (f, reserved, _) =>
      [
        lookahead(
          _first_char_pattern,
          [_first_char_pattern],
          _match_subsequent_chars(f, reserved),
        ),
        lookahead(
          _first_char_pattern,
          [Not(_first_char_pattern)],
          /* only needed here as there are no 1-character reserved tokens */
          s =>
          List.mem(s, reserved) ? empty() : f(s)
        ),
      ]
      |> many
  );

let identifier_matcher =
  Matcher.(
    (~reserved=Knot.Constants.reserved_keywords, ()) => [
      lookahead(_subsequent_char_pattern, [Not(_first_char_pattern)], s =>
        Identifier(s) |> result
      ),
      lookahead(
        _subsequent_char_pattern,
        [_first_char_pattern],
        _match_subsequent_chars(x => Identifier(x) |> result, reserved),
      ),
    ]
  );

let sidecar_matcher =
  Matcher.(
    lookahead(
      Match.dollar_sign,
      [_first_char_pattern],
      _match_subsequent_chars(
        x =>
          SidecarIdentifier(String.sub(x, 1, String.length(x) - 1))
          |> Matcher.result,
        [],
      ),
    )
  );

let matchers = [sidecar_matcher, ...identifier_matcher()];
