open Core;
open Match;
open Matcher;

let matchers = [
  Matcher(
    quote,
    Util.match_until_char(
      quote,
      cursor => UnclosedString(cursor),
      get_string => {
        let string = get_string();

        String(String.sub(string, 1, String.length(string) - 2)) |> result;
      },
    ),
  ),
];
