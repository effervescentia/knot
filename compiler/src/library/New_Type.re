type trait_t =
  | Number
  | Unknown;

type abstract_t('a) = [ | `Abstract('a)];

type container_t('a) = [
  | `List('a)
  | `Struct(list((string, 'a)))
  | `Function(list((string, 'a)), 'a)
];

type primitive_t = [
  | `Nil
  | `Boolean
  | `Integer
  | `Float
  | `String
  | `Element
];

type t = [ primitive_t | container_t(t) | abstract_t(trait_t)];
