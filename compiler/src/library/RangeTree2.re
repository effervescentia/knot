/**
 represents a hierarchy of values and associated ranges
 */
type t('a) =
  | Empty
  | Node('a, Range.t, list(t('a)));

/* static */

let create = (): t('a) => Empty;
