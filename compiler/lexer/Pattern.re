open Core;

let (==>) = Matcher.((s, t) => glyph(s, _ => result(t)));

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
