open Infix;

type t('a) =
  | Empty
  | Node('a, Point.t, t('a), t('a));

/* static */

let create = (): t('a) => Empty;

/* methods */
