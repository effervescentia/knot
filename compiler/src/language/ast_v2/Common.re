open Knot.Kore;

type raw_t('a) = Node.t('a, unit);

type identifier_t = raw_t(string);
