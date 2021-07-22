open Infix;
open Reference;

module Error = {
  type t =
    | ExternalNotFound(Namespace.t, Export.t);
};

type trait_t =
  | Number
  | Unknown;

type abstract_t('a) = [ | `Abstract('a)];

type invalid_t = [ | `Invalid(Error.t)];

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
  type t = [
    | `Strong(strong_t)
    | `Weak(ref(result(weak_t, type_err)))
    | invalid_t
  ]

  and strong_t = [ primitive_t | container_t(t)]

  and weak_t = [ primitive_t | container_t(t) | abstract_t(trait_t)];
};

module Result = {
  type ok_t = [ primitive_t | container_t(ok_t)];

  type t = [ invalid_t | ok_t];
};

/* methods */

let rec to_raw = (type_: t): Raw.t =>
  switch (type_) {
  | `Nil as t
  | `Boolean as t
  | `Integer as t
  | `Float as t
  | `String as t
  | `Element as t => `Strong(t)

  | `List(x) => `Strong(`List(to_raw(x)))
  | `Struct(xs) => `Strong(`Struct(xs |> List.map(Tuple.map_snd2(to_raw))))
  | `Function(args, res) =>
    `Strong(
      `Function((args |> List.map(Tuple.map_snd2(to_raw)), to_raw(res))),
    )

  | `Abstract(x) => `Weak(ref(Ok(`Abstract(x))))
  };

let _err_to_string =
  fun
  | ExternalNotFound(namespace, id) =>
    [
      "ExternalNotFound<" |> Pretty.string,
      namespace |> Namespace.to_string |> Pretty.string,
      "#" |> Pretty.string,
      id |> Export.to_string |> Pretty.string,
      ">" |> Pretty.string,
    ]
    |> Pretty.concat;

let err_to_string = _err_to_string % Pretty.to_string;
