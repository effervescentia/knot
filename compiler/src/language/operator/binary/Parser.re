open Knot.Kore;
open Parse.Kore;

/*
  parsers may have a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* logical */

let rec logical_0 = next => chainl1(logical_1(next), KLogicalOr.Plugin.parse)

and logical_1 = next => chainl1(next, KLogicalAnd.Plugin.parse);

let logical = logical_0;

/* comparison */

let comparison = next =>
  chainl1(next, KEqual.Plugin.parse <|> KUnequal.Plugin.parse);

/* relational */

let relational = next =>
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

let rec arithmetic_0 = next =>
  chainl1(arithmetic_1(next), KAdd.Plugin.parse <|> KSubtract.Plugin.parse)

and arithmetic_1 = next =>
  chainl1(
    arithmetic_2(next),
    KMultiply.Plugin.parse <|> KDivide.Plugin.parse,
  )

and arithmetic_2 = next => chainr1(next, KExponentiate.Plugin.parse);

let arithmetic = arithmetic_0;
