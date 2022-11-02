/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Knot.Kore;
open Reference;
open Common;

include Factory.Make({
  type type_t = Type.t;

  let node_to_entity = (~attributes=[], ~children=[], label, node) =>
    Dump.node_to_entity(Type.pp, ~attributes, ~children, label, node);
});

/**
 a declaration AST node
 */
type declaration_t = node_t(raw_declaration_t)
/**
 supported module declarations
 */
and raw_declaration_t =
  | Constant(expression_t)
  | Enumerated(list((identifier_t, list(node_t(TypeExpression.raw_t)))))
  | Function(list(argument_t), expression_t)
  | View(list(argument_t), list(Node.t(string, Type.t)), expression_t);

/**
 a stdlib import AST node
 */
type standard_import_t = untyped_t((identifier_t, option(identifier_t)));

/**
 an import AST node
 */
type import_t = untyped_t(raw_import_t)
/**
 supported import types
 */
and raw_import_t =
  | MainImport(identifier_t)
  | NamedImport(identifier_t, option(identifier_t));

/**
 supported export types
 */
type export_t =
  | MainExport(identifier_t)
  | NamedExport(identifier_t);

/**
 module statement AST node
 */
type module_statement_t = untyped_t(raw_module_statement_t)
/**
 supported top-level module statements
 */
and raw_module_statement_t =
  | StandardImport(list(standard_import_t))
  | Import(Namespace.t, list(import_t))
  | Declaration(export_t, declaration_t);

/**
 the AST of an entire module
 */
type program_t = list(module_statement_t);

/* tag helpers */

let of_main_import = x => MainImport(x);
let of_named_import = ((x, y)) => NamedImport(x, y);

let of_main_export = x => MainExport(x);
let of_named_export = x => NamedExport(x);

let of_const = x => Constant(x);
let of_enum = variants => Enumerated(variants);
let of_func = ((args, expr)) => Function(args, expr);
let of_view = ((props, mixins, expr)) => View(props, mixins, expr);

let of_standard_import = imports => StandardImport(imports);
let of_import = ((namespace, imports)) => Import(namespace, imports);
let of_decl = ((name, x)) => Declaration(name, x);

module Dump = {
  include Dump;

  let export_to_entity =
    fun
    | MainExport(id) => id_to_entity("MainExport", id)
    | NamedExport(id) => id_to_entity("NamedExport", id);

  let argument_to_entity = (label, arg) => {
    let {name, default, type_} = fst(arg);
    let children = ref([id_to_entity("Name", name)]);

    default
    |> Option.iter(default =>
         children := [expr_to_entity(default), ...children^]
       );

    type_
    |> Option.iter(type_ =>
         children := [untyped_node_to_entity("Type", type_), ...children^]
       );

    typed_node_to_entity(~children=children^, label, arg);
  };

  let decl_to_entity: declaration_t => Entity.t =
    decl =>
      switch (fst(decl)) {
      | Constant(expr) =>
        typed_node_to_entity(
          ~children=[expr_to_entity(expr)],
          "Constant",
          decl,
        )

      | Enumerated(variants) =>
        typed_node_to_entity(
          ~children=
            variants
            |> List.map(((name, args)) =>
                 Entity.create(
                   ~attributes=[("name", fst(name))],
                   ~children=
                     args
                     |> List.map(arg =>
                          typed_node_to_entity(
                            ~children=[
                              N.drop_type(arg)
                              |> TypeExpression.Dump.to_entity,
                            ],
                            "Argument",
                            arg,
                          )
                        ),
                   "Variant",
                 )
               ),
          "Enumerated",
          decl,
        )

      | Function(args, expr) =>
        typed_node_to_entity(
          ~children=[
            Entity.create(
              ~children=args |> List.map(argument_to_entity("Argument")),
              "Arguments",
            ),
            Entity.create(~children=[expr_to_entity(expr)], "Body"),
          ],
          "Function",
          decl,
        )

      | View(props, mixins, expr) =>
        typed_node_to_entity(
          ~children=[
            Entity.create(
              ~children=props |> List.map(argument_to_entity("Property")),
              "Properties",
            ),
            Entity.create(
              ~children=mixins |> List.map(node_to_entity(Type.pp, "Mixin")),
              "Mixins",
            ),
            Entity.create(~children=[expr_to_entity(expr)], "Body"),
          ],
          "View",
          decl,
        )
      };

  let import_to_entity = import =>
    switch (fst(import)) {
    | MainImport(name) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "MainImport",
        import,
      )

    | NamedImport(name, Some(alias)) =>
      untyped_node_to_entity(
        ~children=[
          id_to_entity("Name", name),
          Entity.create(
            ~range=N.get_range(alias),
            ~attributes=[("value", fst(alias))],
            "Alias",
          ),
        ],
        "NamedImport",
        import,
      )

    | NamedImport(name, None) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "NamedImport",
        import,
      )
    };

  let standard_import_to_entity = import =>
    switch (fst(import)) {
    | (name, Some(alias)) =>
      untyped_node_to_entity(
        ~children=[
          id_to_entity("Name", name),
          Entity.create(
            ~range=N.get_range(alias),
            ~attributes=[("value", fst(alias))],
            "Alias",
          ),
        ],
        "Import",
        import,
      )

    | (name, None) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "Import",
        import,
      )
    };

  let mod_stmt_to_entity = mod_stmt =>
    switch (fst(mod_stmt)) {
    | StandardImport(imports) =>
      untyped_node_to_entity(
        ~children=imports |> List.map(standard_import_to_entity),
        "StandardImport",
        mod_stmt,
      )

    | Import(namespace, imports) =>
      untyped_node_to_entity(
        ~attributes=[("namespace", Namespace.to_string(namespace))],
        ~children=imports |> List.map(import_to_entity),
        "Import",
        mod_stmt,
      )

    | Declaration(name, decl) =>
      untyped_node_to_entity(
        ~children=[export_to_entity(name), decl_to_entity(decl)],
        "Declaration",
        mod_stmt,
      )
    };

  let to_entity = program =>
    Entity.create(~children=program |> List.map(mod_stmt_to_entity), "AST");

  let pp: Fmt.t(program_t) = ppf => to_entity % Entity.pp(ppf);
};
