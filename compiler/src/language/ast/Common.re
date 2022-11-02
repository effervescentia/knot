/**
 common types that can be used to build resolved or Raw ASTs
 */
open Knot.Kore;

include Primitive;
include Operator;

module N = Node;

type untyped_t('a) = N.t('a, unit);

/**
 an identifier that doesn't have an inherent type
  */
type identifier_t = untyped_t(string);

/**
 utilities for printing an AST
  */
module Dump = {
  open Pretty.Formatters;

  include Primitive.Dump;
  include Operator.Dump;

  module Entity = {
    type t = {
      name: string,
      range: option(Range.t),
      attributes: list(xml_attr_t(string)),
      children: list(t),
    };

    /* static */

    let create = (~attributes=[], ~children=[], ~range=?, name): t => {
      name,
      range,
      attributes,
      children,
    };

    /* methods */

    let rec to_xml = ({name, range, attributes, children}: t) =>
      Node(
        switch (range) {
        | Some(range) => Fmt.str("%s@%a", name, Range.pp, range)
        | None => name
        },
        attributes,
        children |> List.map(to_xml),
      );

    /* pretty printing */

    let pp: Fmt.t(t) = ppf => to_xml % xml(string, ppf);
  };

  let _attributes_with_type = (type_, pp_type, attributes) => [
    ("type", type_ |> ~@pp_type),
    ...attributes,
  ];

  let untyped_node_to_entity =
      (~attributes=[], ~children=[], label, raw_node: untyped_t('a)) =>
    Entity.create(
      ~range=N.get_range(raw_node),
      ~attributes,
      ~children,
      label,
    );

  let node_to_entity =
      (
        pp_type: Fmt.t('b),
        ~attributes=[],
        ~children=[],
        label,
        node: N.t('a, 'b),
      ) =>
    Entity.create(
      ~range=N.get_range(node),
      ~attributes=
        _attributes_with_type(N.get_type(node), pp_type, attributes),
      ~children,
      label,
    );

  let id_to_entity = (name, id) =>
    untyped_node_to_entity(~attributes=[("value", fst(id))], name, id);
};
