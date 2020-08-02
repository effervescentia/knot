open Globals;
open Match;
open Matcher;

let _first_char = Any([underscore, alpha]);
let _subsequent_char = Any([underscore, alphanumeric]);

let _match_subsequent_chars = (t, _) =>
  Util.match_while(_subsequent_char, t) |> many;

let _single_char_identifier_matcher =
  LookaheadMatcher(
    _first_char,
    [Not(_subsequent_char)],
    get_string => Identifier(get_string()) |> result,
  );

let _identifier_matcher =
  LookaheadMatcher(
    _first_char,
    [_subsequent_char],
    _match_subsequent_chars(get_string => {
      let string = get_string();

      List.mem(string, Knot.Constants.reserved_keywords)
        ? empty : Identifier(string) |> result;
    }),
  );

let _sidecar_matcher =
  LookaheadMatcher(
    dollar_sign,
    [_subsequent_char],
    _match_subsequent_chars(get_string => {
      let string = get_string();

      SidecarIdentifier(String.sub(string, 1, String.length(string) - 1))
      |> result;
    }),
  );

let matchers = [
  _sidecar_matcher,
  _single_char_identifier_matcher,
  _identifier_matcher,
];
