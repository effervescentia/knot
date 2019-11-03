open Core;

let _identifier_matchers = [Constants.underscore, AlphaNumeric];

let rec _match_subsequent_chars = (f, reserved) =>
  [
    LookaheadMatcher(
      Either(_identifier_matchers),
      Either(_identifier_matchers),
      _ => _match_subsequent_chars(f, reserved),
    ),
    LookaheadMatcher(
      Either(_identifier_matchers),
      Except(_identifier_matchers),
      /* only needed here as there are no 1-character reserved tokens */
      s => List.mem(s, reserved) ? empty_matchers : f(s),
    ),
  ]
  |> matcher_list;

let identifier_matcher = (~reserved=Knot.Constants.reserved_keywords, ()) => [
  LookaheadMatcher(
    Either([Constants.underscore, Alpha]),
    Except(_identifier_matchers),
    s => result(Identifier(s)),
  ),
  LookaheadMatcher(
    Either([Constants.underscore, Alpha]),
    Either(_identifier_matchers),
    _ => _match_subsequent_chars(x => result(Identifier(x)), reserved),
  ),
];

let sidecar_matcher =
  LookaheadMatcher(
    Constants.dollar_sign,
    Either(_identifier_matchers),
    _ =>
      _match_subsequent_chars(
        x =>
          result(
            SidecarIdentifier(String.sub(x, 1, String.length(x) - 1)),
          ),
        [],
      ),
  );

let matchers = [sidecar_matcher, ...identifier_matcher()];
