open Infix;
open Reference;

module Trait = {
  type t =
    | Number
    | Unknown;

  let to_string =
    fun
    | Number => "Number"
    | Unknown => "Unknown";
};

module Error = {
  type t('a) =
    | TypeMismatch('a, 'a)
    | NotAssignable('a, Trait.t)
    | ExternalNotFound(Namespace.t, Export.t)
    | TypeResolutionFailed;

  let to_string = (type_to_string: 'a => string) =>
    fun
    | NotAssignable(type_, trait) =>
      Print.fmt(
        "NotAssignable<%s, %s>",
        type_to_string(type_),
        Trait.to_string(trait),
      )
    | TypeMismatch(lhs, rhs) =>
      Print.fmt(
        "TypeMismatch<%s, %s>",
        type_to_string(lhs),
        type_to_string(rhs),
      )
    | TypeResolutionFailed => "TypeResolutionFailed"
    | ExternalNotFound(namespace, id) =>
      Print.fmt(
        "ExternalNotFound<%s#%s>",
        Namespace.to_string(namespace),
        Export.to_string(id),
      );
};

type abstract_t('a) = [ | `Abstract('a)];

type invalid_t('a) = [ | `Invalid(Error.t('a))];

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

type t = [ primitive_t | container_t(t) | abstract_t(Trait.t)];

module Raw = {
  type t = [
    | `Strong(strong_t)
    | `Weak(ref(result(weak_t, error_t)))
    | invalid_t(strong_t)
  ]

  and error_t = Error.t(strong_t)

  and strong_t = [ primitive_t | container_t(t)]

  and weak_t = [ primitive_t | container_t(t) | abstract_t(Trait.t)];

  let primitive_to_string = (type_: primitive_t) =>
    Constants.(
      switch (type_) {
      | `Nil => Keyword.nil
      | `Boolean => "bool"
      | `Integer => "int"
      | `Float => "float"
      | `String => "string"
      | `Element => "element"
      }
    );

  let list_to_string = (type_to_string: 'a => string, t: 'a) =>
    t |> type_to_string |> Print.fmt("List<%s>");

  let struct_to_string =
      (type_to_string: 'a => string, props: list((string, 'a))) =>
    List.is_empty(props)
      ? "{}"
      : props
        |> List.map(
             Tuple.map_snd2(type_to_string)
             % Tuple.reduce2(Print.fmt("%s: %s")),
           )
        |> List.intersperse(", ")
        |> String.join
        |> Print.fmt("{ %s }");

  let function_to_string =
      (type_to_string: 'a => string, args: list((string, 'a)), res: 'a) =>
    Print.fmt(
      "Function<(%s), %s>",
      args
      |> List.map(
           Tuple.map_snd2(type_to_string)
           % Tuple.reduce2(Print.fmt("%s: %s")),
         )
      |> List.intersperse(", ")
      |> String.join,
      type_to_string(res),
    );

  let rec to_string = (type_: t) =>
    switch (type_) {
    | `Strong(t) => strong_to_string(t)
    | `Invalid(err) => err |> Error.to_string(strong_to_string)
    | `Weak({contents: weak_type}) =>
      (
        switch (weak_type) {
        | Ok(ok_type) =>
          switch (ok_type) {
          | `Nil as x
          | `Boolean as x
          | `Integer as x
          | `Float as x
          | `String as x
          | `Element as x => primitive_to_string(x)
          | `List(t) => t |> list_to_string(to_string)
          | `Struct(props) => props |> struct_to_string(to_string)
          | `Function(args, res) => function_to_string(to_string, args, res)
          | `Abstract(trait) =>
            trait |> Trait.to_string |> Print.fmt("Abstract<%s>")
          }
        | Error(err) => err |> Error.to_string(strong_to_string)
        }
      )
      |> Print.fmt("Weak<%s>")
    }

  and strong_to_string = (type_: strong_t) =>
    Constants.(
      switch (type_) {
      | `Nil as x
      | `Boolean as x
      | `Integer as x
      | `Float as x
      | `String as x
      | `Element as x => primitive_to_string(x)
      | `List(t) => t |> list_to_string(to_string)
      | `Struct(props) => props |> struct_to_string(to_string)
      | `Function(args, res) => function_to_string(to_string, args, res)
      }
    );
};

module Result = {
  type valid_t = [ primitive_t | container_t(valid_t)];

  type t = [ | `Valid(valid_t) | invalid_t(valid_t)];

  /* methods */

  let rec _valid_to_raw = (type_: valid_t): Raw.t =>
    `Strong(_valid_to_strong(type_))

  and _valid_to_strong = (type_: valid_t): Raw.strong_t =>
    switch (type_) {
    | `Nil as t
    | `Boolean as t
    | `Integer as t
    | `Float as t
    | `String as t
    | `Element as t => t

    | `List(x) => `List(_valid_to_raw(x))
    | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(_valid_to_raw)))
    | `Function(args, res) =>
      `Function((
        args |> List.map(Tuple.map_snd2(_valid_to_raw)),
        _valid_to_raw(res),
      ))
    };

  let rec to_raw = (type_: t): Raw.t =>
    switch (type_) {
    | `Valid(t) => _valid_to_raw(t)
    | `Invalid(NotAssignable(t, trait)) =>
      `Invalid(NotAssignable(_valid_to_strong(t), trait))
    | `Invalid(ExternalNotFound(_) as err) => `Invalid(err)
    }

  and err_to_strong_err = (err: Error.t(valid_t)): Raw.error_t =>
    switch (err) {
    | NotAssignable(t, trait) => NotAssignable(_valid_to_strong(t), trait)
    | ExternalNotFound(_) as err => err
    };
};
