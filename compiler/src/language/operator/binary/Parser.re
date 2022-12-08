open Kore;
open Parse.Kore;

/*
  parsers may have a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* logical */

let rec parse_logical_0 = next =>
  chainl1(parse_logical_1(next), KLogicalOr.parse)

and parse_logical_1 = next => chainl1(next, KLogicalAnd.parse);

let parse_logical = parse_logical_0;

/* comparison */

let parse_comparison = next => chainl1(next, KEqual.parse <|> KUnequal.parse);

/* relational */

let parse_relational = next =>
  chainl1(
    next,
    choice([
      KLessOrEqual.parse,
      KLessThan.parse,
      KGreaterOrEqual.parse,
      KGreaterThan.parse,
    ]),
  );

/* arithmetic */

let rec parse_arithmetic_0 = next =>
  chainl1(parse_arithmetic_1(next), KAdd.parse <|> KSubtract.parse)

and parse_arithmetic_1 = next =>
  chainl1(parse_arithmetic_2(next), KMultiply.parse <|> KDivide.parse)

and parse_arithmetic_2 = next => chainr1(next, KExponentiate.parse);

let parse_arithmetic = parse_arithmetic_0;
