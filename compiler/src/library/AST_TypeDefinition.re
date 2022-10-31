/**
 Type definition module Abstract Syntax Tree.
 */
open Infix;
open AST_Common;

module TypeExpression = AST_TypeExpression;

type variant_t = (untyped_t(string), list(TypeExpression.t));

type decorator_t('a) =
  untyped_t((N.t(string, 'a), list(N.t(primitive_t, 'a))));

type module_statement_t = untyped_t(raw_module_statement_t)

and raw_module_statement_t =
  | Declaration(untyped_t(string), TypeExpression.t)
  | Type(untyped_t(string), TypeExpression.t)
  | Enumerated(untyped_t(string), list(variant_t));

type module_t = untyped_t(raw_module_t)

and raw_module_t =
  | Decorator(
      untyped_t(string),
      list(TypeExpression.t),
      Type.DecoratorTarget.t,
    )
  | Module(
      untyped_t(string),
      list(module_statement_t),
      list(decorator_t(Type.t)),
    );

type t = list(module_t);

/* tag helpers */

let of_decorator = ((id, args, target)) => Decorator(id, args, target);
let of_declaration = ((id, type_)) => Declaration(id, type_);
let of_type = ((id, type_)) => Type(id, type_);
let of_enum = ((id, variants)) => Enumerated(id, variants);
let of_module = ((id, stmts, decorators)) => Module(id, stmts, decorators);

module Dump = {
  include Dump;

  let decorator_to_entity =
      (pp_type: Fmt.t('a), ((id, args), _) as decorator: decorator_t('a)) =>
    decorator
    |> untyped_node_to_entity(
         ~children=[
           id
           |> node_to_entity(
                pp_type,
                ~attributes=[("value", fst(id))],
                "Identifier",
              ),
           ...args
              |> List.map(arg =>
                   arg
                   |> node_to_entity(
                        pp_type,
                        ~attributes=[
                          ("value", arg |> fst |> prim_to_string),
                        ],
                        "Argument",
                      )
                 ),
         ],
         "Decorator",
       );

  let to_entity = module_ =>
    switch (fst(module_)) {
    | Decorator(id, args, target) =>
      untyped_node_to_entity(
        ~attributes=[
          ("id", fst(id)),
          ("target", target |> ~@Type.DecoratorTarget.pp),
        ],
        ~children=
          args
          |> List.map(arg =>
               Entity.create(
                 ~children=[TypeExpression.Dump.to_entity(arg)],
                 "Argument",
               )
             ),
        "Decorator",
        id,
      )

    | Module((name, _), stmts, decorators) =>
      untyped_node_to_entity(
        ~attributes=[("name", name)],
        ~children=
          (decorators |> List.map(decorator_to_entity(Type.pp)))
          @ (
            stmts
            |> List.map(stmt =>
                 stmt
                 |> untyped_node_to_entity(
                      ~children=[
                        switch (fst(stmt)) {
                        | Declaration((id, _), type_) =>
                          Entity.create(
                            ~attributes=[("id", id)],
                            ~children=[TypeExpression.Dump.to_entity(type_)],
                            "Declaration",
                          )

                        | Type((id, _), type_) =>
                          Entity.create(
                            ~attributes=[("id", id)],
                            ~children=[TypeExpression.Dump.to_entity(type_)],
                            "Type",
                          )

                        | Enumerated((id, _), variants) =>
                          Entity.create(
                            ~attributes=[("id", id)],
                            ~children=
                              variants
                              |> List.map(((name, args)) =>
                                   Entity.create(
                                     ~attributes=[("name", fst(name))],
                                     ~children=
                                       args
                                       |> List.map(
                                            TypeExpression.Dump.to_entity,
                                          ),
                                     "Variant",
                                   )
                                 ),
                            "Enumerated",
                          )
                        },
                      ],
                      "Statement",
                    )
               )
          ),
        "Module",
        module_,
      )
    };
};
