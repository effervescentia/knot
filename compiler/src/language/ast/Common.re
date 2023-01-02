/**
 common types that can be used to build resolved or Raw ASTs
 */
open Knot.Kore;

include Primitive;
include Operator;

type untyped_t('a) = Node.t('a, unit);
type raw_t('a) = Node.t('a, unit);

/**
 an identifier that doesn't have an inherent type
  */
type identifier_t = untyped_t(string);
