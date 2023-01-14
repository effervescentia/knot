open Kore;
open Parse.Kore;

/*
  parsers may have a precedence denoted by its suffix

  the parser with the highest precedence should be matched first
 */

/* logical */

let rec parse_logical_0 = (f, next) =>
  chainl1(parse_logical_1(f, next), KLogicalOr.parse(f))

and parse_logical_1 = (f, next) => chainl1(next, KLogicalAnd.parse(f));

let parse_logical = parse_logical_0;

/* comparison */

let parse_comparison = (f, next) =>
  chainl1(next, KEqual.parse(f) <|> KUnequal.parse(f));

/* relational */

let parse_relational = (f, next) =>
  chainl1(
    next,
    choice([
      KLessOrEqual.parse(f),
      KLessThan.parse(f),
      KGreaterOrEqual.parse(f),
      KGreaterThan.parse(f),
    ]),
  );

/* arithmetic */

let rec parse_arithmetic_0 = (f, next) =>
  chainl1(
    parse_arithmetic_1(f, next),
    KAdd.parse(f) <|> KSubtract.parse(f),
  )

and parse_arithmetic_1 = (f, next) =>
  chainl1(
    parse_arithmetic_2(f, next),
    KMultiply.parse(f) <|> KDivide.parse(f),
  )

and parse_arithmetic_2 = (f, next) => chainr1(next, KExponentiate.parse(f));

let parse_arithmetic = parse_arithmetic_0;
