/**
 Type expression Abstract Syntax Tree.
 */
open Infix;
open Reference;
open AST_Common;

type t = untyped_t(raw_t)

and raw_t =
  | Nil
  | Boolean
  | Integer
  | Float
  | String
  | Element
  | Identifier(untyped_t(string))
  | Group(t)
  | List(t)
  | Struct(list((untyped_t(string), t)))
  | Function(list(t), t)
  | DotAccess(t, untyped_t(string));

/* tag helpers */

let of_id = x => Identifier(x);
let of_group = x => Group(x);
let of_list = x => List(x);
let of_struct = props => Struct(props);
let of_function = ((args, res)) => Function(args, res);
let of_dot_access = ((id, prop)) => DotAccess(id, prop);

module Dump = {
  include Dump;

  let rec to_entity = type_ =>
    (
      switch (fst(type_)) {
      | Nil => untyped_node_to_entity("Nil")

      | Boolean => untyped_node_to_entity("Boolean")

      | Integer => untyped_node_to_entity("Integer")

      | Float => untyped_node_to_entity("Float")

      | String => untyped_node_to_entity("String")

      | Element => untyped_node_to_entity("Element")

      | Identifier((name, _)) =>
        untyped_node_to_entity(~attributes=[("name", name)], "Identifier")

      | Group(t) =>
        untyped_node_to_entity(~children=[to_entity(t)], "Group")

      | List(t) => untyped_node_to_entity(~children=[to_entity(t)], "List")

      | Struct(props) =>
        untyped_node_to_entity(
          ~children=
            props
            |> List.map(((key, value)) =>
                 Entity.create(
                   ~children=[
                     untyped_node_to_entity(
                       ~attributes=[("name", fst(key))],
                       "Key",
                       key,
                     ),
                     untyped_node_to_entity(
                       ~children=[to_entity(value)],
                       "Value",
                       value,
                     ),
                   ],
                   "Property",
                 )
               ),
          "Struct",
        )

      | Function(args, res) =>
        untyped_node_to_entity(
          ~children=[
            Entity.create(
              ~children=
                args
                |> List.map(arg =>
                     untyped_node_to_entity(
                       ~children=[to_entity(arg)],
                       "Argument",
                       arg,
                     )
                   ),
              "Arguments",
            ),
            untyped_node_to_entity(
              ~children=[to_entity(res)],
              "Result",
              res,
            ),
          ],
          "Function",
        )

      | DotAccess(root, prop) =>
        untyped_node_to_entity(
          ~children=[
            Entity.create(~children=[to_entity(root)], "Root"),
            prop
            |> untyped_node_to_entity(
                 ~attributes=[("name", fst(prop))],
                 "Property",
               ),
          ],
          "DotAccess",
        )
      }
    )(
      type_,
    );
};
