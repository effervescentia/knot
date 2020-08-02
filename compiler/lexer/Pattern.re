open Globals;
open Match;
open Matcher;

let _glyph = t =>
  Knot.Util.split
  % (
    fun
    | []
    | [_] => invariant(InvalidGlyph)
    | [char, ...next_chars] =>
      Matcher(Exactly(char), Util.match_tentative(t, next_chars))
  );

let (==>) = (s, t) => _glyph(_ => result(t), s);

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
