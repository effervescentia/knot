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

module Raw = {
  open Infix;
  open Reference;

  type type_err = |;

  type t =
    | Strong(strong_t)
    | Weak(ref(result(weak_t, type_err)))
    | Invalid(type_err)

  and strong_t = [ primitive_t | container_t(t)]

  and weak_t = [ primitive_t | container_t(t) | abstract_t(trait_t)];
};

/* methods */

let rec to_raw = (type_: t): Raw.t =>
  switch (type_) {
  | `Nil as t
  | `Boolean as t
  | `Integer as t
  | `Float as t
  | `String as t
  | `Element as t => Strong(t)

  | `List(x) => Strong(`List(to_raw(x)))
  | `Struct(xs) => Strong(`Struct(xs |> List.map(Tuple.map_snd2(to_raw))))
  | `Function(args, res) =>
    Strong(
      `Function((args |> List.map(Tuple.map_snd2(to_raw)), to_raw(res))),
    )

  | `Abstract(x) => Weak(ref(Ok(`Abstract(x))))
  };
