open Knot.Kore;
open Parse.Kore;

/*
  parsers may have a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* logical */

let rec parse_logical_0 = next =>
  chainl1(parse_logical_1(next), KLogicalOr.Plugin.parse)

and parse_logical_1 = next => chainl1(next, KLogicalAnd.Plugin.parse);

let parse_logical = parse_logical_0;

/* comparison */

let parse_comparison = next =>
  chainl1(next, KEqual.Plugin.parse <|> KUnequal.Plugin.parse);

/* relational */

let parse_relational = next =>
  chainl1(
    next,
    choice([
      KLessOrEqual.Plugin.parse,
      KLessThan.Plugin.parse,
      KGreaterOrEqual.Plugin.parse,
      KGreaterThan.Plugin.parse,
    ]),
  );

/* arithmetic */

let rec parse_arithmetic_0 = next =>
  chainl1(
    parse_arithmetic_1(next),
    KAdd.Plugin.parse <|> KSubtract.Plugin.parse,
  )

and parse_arithmetic_1 = next =>
  chainl1(
    parse_arithmetic_2(next),
    KMultiply.Plugin.parse <|> KDivide.Plugin.parse,
  )

and parse_arithmetic_2 = next => chainr1(next, KExponentiate.Plugin.parse);

let parse_arithmetic = parse_arithmetic_0;
