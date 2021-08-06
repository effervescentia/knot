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
    | NotFound(Identifier.t)
    | TypeMismatch('a, 'a)
    | NotAssignable('a, Trait.t)
    | ExternalNotFound(Namespace.t, Export.t)
    | TypeResolutionFailed
    | DuplicateIdentifier(Identifier.t);

  let to_string = (type_to_string: 'a => string) =>
    fun
    | NotFound(id) => id |> Identifier.to_string |> Print.fmt("NotFound<%s>")
    | DuplicateIdentifier(id) =>
      id |> Identifier.to_string |> Print.fmt("DuplicateIdentifier<%s>")
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

module Raw = {
  type t =
    | Strong(strong_t)
    | Weak(ref(result(weak_t, error_t)))
    | Invalid(Error.t(t))

  and error_t = Error.t(t)

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

  let abstract_to_string = (trait: Trait.t) =>
    trait |> Trait.to_string |> Print.fmt("Abstract<%s>");

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
    | Strong(t) => strong_to_string(t)
    | Invalid(err) => err |> Error.to_string(to_string)
    | Weak({contents: weak_type}) =>
      (
        switch (weak_type) {
        | Ok(ok_type) =>
          switch (ok_type) {
          | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
            primitive_to_string(x)
          | `List(t) => t |> list_to_string(to_string)
          | `Struct(props) => props |> struct_to_string(to_string)
          | `Function(args, res) => function_to_string(to_string, args, res)
          | `Abstract(trait) => abstract_to_string(trait)
          }
        | Error(err) => err |> Error.to_string(to_string)
        }
      )
      |> Print.fmt("Weak<%s>")
    }

  and strong_to_string = (type_: strong_t) =>
    switch (type_) {
    | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
      primitive_to_string(x)
    | `List(t) => t |> list_to_string(to_string)
    | `Struct(props) => props |> struct_to_string(to_string)
    | `Function(args, res) => function_to_string(to_string, args, res)
    };
};

type t =
  | Valid(valid_t)
  | Invalid(Error.t(t))

and valid_t = [ primitive_t | container_t(t) | abstract_t(Trait.t)]

and error_t = Error.t(t);

let of_result = (res: result(valid_t, error_t)) =>
  switch (res) {
  | Ok(x) => Valid(x)
  | Error(err) => Invalid(err)
  };

/* methods */

let to_result = (res: t) =>
  switch (res) {
  | Valid(x) => Ok(x)
  | Invalid(err) => Error(err)
  };

let rec _valid_to_raw = (type_: valid_t): Raw.t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t =>
    Strong(t)

  | `List(x) => Strong(`List(to_raw(x)))
  | `Struct(xs) => Strong(`Struct(xs |> List.map(Tuple.map_snd2(to_raw))))
  | `Function(args, res) =>
    Strong(
      `Function((args |> List.map(Tuple.map_snd2(to_raw)), to_raw(res))),
    )
  | `Abstract(_) as x => Weak(ref(Ok(x)))
  }

and to_raw = (type_: t): Raw.t =>
  switch (type_) {
  | Valid(t) => _valid_to_raw(t)
  | Invalid(NotAssignable(t, trait)) =>
    Invalid(NotAssignable(to_raw(t), trait))
  | Invalid(TypeMismatch(lhs, rhs)) =>
    Invalid(TypeMismatch(to_raw(lhs), to_raw(rhs)))
  | Invalid(
      (
        ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
        TypeResolutionFailed
      ) as err,
    ) =>
    Invalid(err)
  }

and err_to_strong_err = (err: error_t): Raw.error_t =>
  switch (err) {
  | NotAssignable(t, trait) => NotAssignable(to_raw(t), trait)
  | TypeMismatch(lhs, rhs) => TypeMismatch(to_raw(lhs), to_raw(rhs))
  | (
      ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
      TypeResolutionFailed
    ) as err => err
  };

let rec to_string = (type_: t) =>
  switch (type_) {
  | Valid(t) => valid_to_string(t)
  | Invalid(err) => err |> Error.to_string(to_string)
  }

and valid_to_string = (type_: valid_t) =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as x =>
    Raw.primitive_to_string(x)
  | `List(t) => t |> Raw.list_to_string(to_string)
  | `Struct(props) => props |> Raw.struct_to_string(to_string)
  | `Function(args, res) => Raw.function_to_string(to_string, args, res)
  | `Abstract(trait) => Raw.abstract_to_string(trait)
  };

let rec of_raw = (type_: Raw.t): t =>
  switch (type_) {
  | Strong(t) => Valid(valid_of_strong(t))
  | Weak({contents: Ok(t)}) => Valid(valid_of_weak(t))
  | Weak({contents: Error(err)}) => Invalid(err_of_raw_err(err))
  | Invalid(err) => Invalid(err_of_raw_err(err))
  }

and valid_of_strong = (type_: Raw.strong_t): valid_t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
  | `List(x) => `List(of_raw(x))
  | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
  | `Function(args, res) =>
    `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
  }

and valid_of_weak = (type_: Raw.weak_t): valid_t =>
  switch (type_) {
  | (`Nil | `Boolean | `Integer | `Float | `String | `Element) as t => t
  | `List(x) => `List(of_raw(x))
  | `Struct(xs) => `Struct(xs |> List.map(Tuple.map_snd2(of_raw)))
  | `Function(args, res) =>
    `Function((args |> List.map(Tuple.map_snd2(of_raw)), of_raw(res)))
  | `Abstract(t) => `Abstract(t)
  }

and err_of_raw_err = (err: Raw.error_t): error_t =>
  switch (err) {
  | TypeMismatch(lhs, rhs) => TypeMismatch(of_raw(lhs), of_raw(rhs))
  | NotAssignable(x, y) => NotAssignable(of_raw(x), y)
  | (
      ExternalNotFound(_) | DuplicateIdentifier(_) | NotFound(_) |
      TypeResolutionFailed
    ) as err => err
  };
