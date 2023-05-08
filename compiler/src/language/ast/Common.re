/**
 common types that can be used to build resolved or Raw ASTs
 */
open Knot.Kore;

type raw_t('a) = Node.t('a, unit);

/**
 an identifier that doesn't have an inherent type
  */
type identifier_t = raw_t(string);
