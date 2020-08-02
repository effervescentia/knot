open Globals;
open Match;
open Matcher;

let matchers =
  Util.match_while(numeric, get_string =>
    Number(int_of_string(get_string())) |> result
  );
