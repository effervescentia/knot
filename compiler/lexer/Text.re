open Core;

let rec match_string = _ =>
  [
    TerminalMatcher(
      UnclosedString,
      Constants.quote,
      s => result(String(String.sub(s, 1, String.length(s) - 2))),
    ),
    TerminalMatcher(UnclosedString, Except([Constants.quote]), match_string),
  ]
  |> matcher_list;

let matchers = [Matcher(Constants.quote, match_string)];
