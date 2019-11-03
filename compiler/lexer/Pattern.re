open Core;

let rec _permiss = (matcher, match_skip) =>
  [matcher, Matcher(match_skip, _ => _permiss(matcher, match_skip))]
  |> matcher_list;

let rec (==>) = (p, t) => {
  let next = _ =>
    if (String.length(p) == 1) {
      result(t);
    } else {
      _permiss(
        String.sub(p, 1, String.length(p) - 1) ==> t,
        Constants.whitespace,
      );
    };

  Matcher(Char(p.[0]), next);
};

let matchers = [
  "&&" ==> LogicalAnd,
  "||" ==> LogicalOr,
  "->" ==> Lambda,
  "==" ==> Equals,
  "!=" ==> NotEquals,
  "<=" ==> LessThanOrEqual,
  ">=" ==> GreaterThanOrEqual,
  "/>" ==> JSXSelfClose,
  "</" ==> JSXOpenEnd,
  "<>" ==> JSXStartFragment,
  "</>" ==> JSXEndFragment,
];
