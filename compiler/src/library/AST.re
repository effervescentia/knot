/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Infix;
open Reference;

/**
 common types that can be used to build resolved or Raw ASTs
 */
module Common = {
  /**
   supported binary operators
   */
  type binary_operator_t =
    /* logical operators */
    | LogicalAnd
    | LogicalOr
    /* comparative operators */
    | LessOrEqual
    | LessThan
    | GreaterOrEqual
    | GreaterThan
    /* equality operators */
    | Equal
    | Unequal
    /* arithmetic operators */
    | Add
    | Subtract
    | Divide
    | Multiply
    | Exponent;

  /**
   supported unary operators
   */
  type unary_operator_t =
    | Not
    | Positive
    | Negative;

  /**
   supported numeric types
   */
  type number_t =
    | Integer(Int64.t)
    | Float(float, int);

  /**
   an identifier that doesn't have an inherent type
   */
  type untyped_identifier_t = Node.Raw.t(Identifier.t);

  /**
   utilities for printing an AST
   */
  module Dump = {
    open Pretty.Formatters;

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

    let raw_node_to_entity = (~attributes=[], ~children=[], label, raw_node) =>
      Entity.create(
        ~range=Node.Raw.get_range(raw_node),
        ~attributes,
        ~children,
        label,
      );

    let analyzed_node_to_entity = (~attributes=[], ~children=[], label, node) =>
      Entity.create(
        ~range=Node.get_range(node),
        ~attributes=
          _attributes_with_type(
            Node.get_type(node),
            Type.Raw.pp,
            attributes,
          ),
        ~children,
        label,
      );

    let node_to_entity = (~attributes=[], ~children=[], label, node) =>
      Entity.create(
        ~range=Node.get_range(node),
        ~attributes=
          _attributes_with_type(Node.get_type(node), Type.pp, attributes),
        ~children,
        label,
      );
  };
};

/**
 abstraction on the type of the nodes that makeup an AST
 */
module type ASTParams = {
  type type_t;

  type node_t('a);

  let get_value: node_t('a) => 'a;
  let get_range: node_t('a) => Range.t;

  let node_to_entity:
    (
      ~attributes: list(Pretty.XML.xml_attr_t(string))=?,
      ~children: list(Common.Dump.Entity.t)=?,
      string,
      node_t('a)
    ) =>
    Common.Dump.Entity.t;
};

/**
 constructor for AST modules
 */
module Make = (T: ASTParams) => {
  include Common;

  /**
   type container for AST nodes
   */
  type type_t = T.type_t;

  /**
   an identifier AST node
   */
  type identifier_t = T.node_t(Identifier.t);

  /**
   a primitive AST node
   */
  type primitive_t = T.node_t(raw_primitive_t)
  /**
   supported primitive types
   */
  and raw_primitive_t =
    | Nil
    | Boolean(bool)
    | Number(number_t)
    | String(string);

  /**
   a JSX AST node
   */
  type jsx_t = T.node_t(raw_jsx_t)
  /**
   supported top-level JSX structures
   */
  and raw_jsx_t =
    | Tag(untyped_identifier_t, list(jsx_attribute_t), list(jsx_child_t))
    | Fragment(list(jsx_child_t))

  /**
   a JSX child AST node
   */
  and jsx_child_t = T.node_t(raw_jsx_child_t)
  /**
   supported JSX children
   */
  and raw_jsx_child_t =
    | Text(T.node_t(string))
    | Node(jsx_t)
    | InlineExpression(expression_t)

  /**
   a JSX attribute AST node
   */
  and jsx_attribute_t = T.node_t(raw_jsx_attribute_t)
  /**
   supported JSX attributes
   */
  and raw_jsx_attribute_t =
    | ID(untyped_identifier_t)
    | Class(untyped_identifier_t, option(expression_t))
    | Property(untyped_identifier_t, option(expression_t))

  /**
   an expression AST node
   */
  and expression_t = T.node_t(raw_expression_t)
  /**
   supported expressions and type containers
   */
  and raw_expression_t =
    | Primitive(primitive_t)
    | Identifier(identifier_t)
    | JSX(jsx_t)
    | Group(expression_t)
    | BinaryOp(binary_operator_t, expression_t, expression_t)
    | UnaryOp(unary_operator_t, expression_t)
    | Closure(list(statement_t))

  /**
   a statement AST node
   */
  and statement_t = T.node_t(raw_statement_t)
  /**
   supported statement types
   */
  and raw_statement_t =
    | Variable(untyped_identifier_t, expression_t)
    | Expression(expression_t);

  /**
   an AST node of an argument for a functional closure
   */
  type argument_t = T.node_t(raw_argument_t)
  /**
   a node of an argument for a functional closure
   */
  and raw_argument_t = {
    name: untyped_identifier_t,
    default: option(expression_t),
    type_: option(T.node_t(Type.t)),
  };

  /* tag helpers */

  let of_internal = namespace => Namespace.Internal(namespace);
  let of_external = namespace => Namespace.External(namespace);

  let of_public = name => Identifier.Public(name);
  let of_private = name => Identifier.Private(name);

  let of_var = ((name, x)) => Variable(name, x);
  let of_expr = x => Expression(x);
  let of_id = x => Identifier(x);
  let of_group = x => Group(x);
  let of_closure = xs => Closure(xs);

  let of_not_op = x => UnaryOp(Not, x);
  let of_neg_op = x => UnaryOp(Negative, x);
  let of_pos_op = x => UnaryOp(Positive, x);

  let of_and_op = ((l, r)) => BinaryOp(LogicalAnd, l, r);
  let of_or_op = ((l, r)) => BinaryOp(LogicalOr, l, r);

  let of_mult_op = ((l, r)) => BinaryOp(Multiply, l, r);
  let of_div_op = ((l, r)) => BinaryOp(Divide, l, r);
  let of_add_op = ((l, r)) => BinaryOp(Add, l, r);
  let of_sub_op = ((l, r)) => BinaryOp(Subtract, l, r);

  let of_lt_op = ((l, r)) => BinaryOp(LessThan, l, r);
  let of_lte_op = ((l, r)) => BinaryOp(LessOrEqual, l, r);
  let of_gt_op = ((l, r)) => BinaryOp(GreaterThan, l, r);
  let of_gte_op = ((l, r)) => BinaryOp(GreaterOrEqual, l, r);

  let of_eq_op = ((l, r)) => BinaryOp(Equal, l, r);
  let of_ineq_op = ((l, r)) => BinaryOp(Unequal, l, r);

  let of_expo_op = ((l, r)) => BinaryOp(Exponent, l, r);

  let of_jsx = x => JSX(x);
  let of_frag = xs => Fragment(xs);
  let of_tag = ((name, attrs, children)) => Tag(name, attrs, children);
  let of_prop = ((name, value)) => Property(name, value);
  let of_jsx_class = ((name, value)) => Class(name, value);
  let of_jsx_id = name => ID(name);
  let of_text = x => Text(x);
  let of_node = x => Node(x);
  let of_inline_expr = x => InlineExpression(x);

  let of_prim = x => Primitive(x);
  let of_bool = x => Boolean(x);
  let of_int = x => Integer(x);
  let of_float = ((x, precision)) => Float(x, precision);
  let of_string = x => String(x);
  let of_num = x => Number(x);
  let nil = Nil;

  module Dump = {
    open Pretty.Formatters;

    include Common.Dump;

    let id_to_entity = (name, id) =>
      raw_node_to_entity(
        ~attributes=[
          ("value", id |> Node.Raw.get_value |> Identifier.to_string),
        ],
        name,
        id,
      );

    let num_to_string =
      fun
      | Integer(int) => int |> Int64.to_string
      | Float(float, precision) => float |> Fmt.str("%.*f", precision);

    let prim_to_entity = prim =>
      (
        switch (T.get_value(prim)) {
        | Nil => T.node_to_entity("Nil")

        | Boolean(bool) =>
          T.node_to_entity(
            ~attributes=[("value", string_of_bool(bool))],
            "Boolean",
          )

        | Number(num) =>
          T.node_to_entity(
            ~attributes=[("value", num_to_string(num))],
            "Number",
          )

        | String(str) =>
          T.node_to_entity(
            ~attributes=[("value", str |> ~@Fmt.quote(string))],
            "String",
          )
        }
      )(
        prim,
      );

    let rec expr_to_entity = expr =>
      (
        switch (T.get_value(expr)) {
        | Primitive(prim) =>
          T.node_to_entity(~children=[prim_to_entity(prim)], "Primitive")

        | Identifier(id) =>
          T.node_to_entity(
            ~attributes=[
              ("value", id |> T.get_value |> Identifier.to_string),
            ],
            "Identifier",
          )

        | JSX(jsx) =>
          T.node_to_entity(~children=[jsx_to_entity(jsx)], "JSX")

        | Group(group) =>
          T.node_to_entity(~children=[expr_to_entity(group)], "Group")

        | Closure(stmts) =>
          T.node_to_entity(
            ~children=
              stmts
              |> List.map(stmt =>
                   T.node_to_entity(
                     ~children=[stmt_to_entity(stmt)],
                     "Statement",
                     stmt,
                   )
                 ),
            "Closure",
          )

        | BinaryOp(op, lhs, rhs) =>
          T.node_to_entity(
            ~children=[
              T.node_to_entity(~children=[expr_to_entity(lhs)], "LHS", lhs),
              T.node_to_entity(~children=[expr_to_entity(rhs)], "RHS", rhs),
            ],
            switch (op) {
            | LogicalAnd => "And"
            | LogicalOr => "Or"
            | Add => "Add"
            | Subtract => "Sub"
            | Divide => "Div"
            | Multiply => "Mult"
            | LessOrEqual => "LessOrEq"
            | LessThan => "Less"
            | GreaterOrEqual => "GreaterOrEq"
            | GreaterThan => "Greater"
            | Equal => "Equal"
            | Unequal => "Unequal"
            | Exponent => "Exponent"
            },
          )

        | UnaryOp(op, expr) =>
          T.node_to_entity(
            ~children=[expr_to_entity(expr)],
            switch (op) {
            | Not => "Not"
            | Positive => "Positive"
            | Negative => "Negative"
            },
          )
        }
      )(
        expr,
      )

    and jsx_to_entity = jsx =>
      (
        switch (T.get_value(jsx)) {
        | Tag(name, attrs, children) =>
          T.node_to_entity(
            ~children=[
              id_to_entity("Name", name),
              Entity.create(
                ~children=attrs |> List.map(jsx_attr_to_entity),
                "Attributes",
              ),
              Entity.create(
                ~children=children |> List.map(jsx_child_to_entity),
                "Children",
              ),
            ],
            "Tag",
          )

        | Fragment(children) =>
          T.node_to_entity(
            ~children=children |> List.map(jsx_child_to_entity),
            "Fragment",
          )
        }
      )(
        jsx,
      )

    and jsx_child_to_entity = jsx_child =>
      (
        switch (T.get_value(jsx_child)) {
        | Text(text) =>
          T.node_to_entity(
            ~attributes=[("value", T.get_value(text))],
            "Text",
          )

        | Node(jsx) =>
          T.node_to_entity(~children=[jsx_to_entity(jsx)], "Node")

        | InlineExpression(expr) =>
          T.node_to_entity(~children=[expr_to_entity(expr)], "InlineExpr")
        }
      )(
        jsx_child,
      )

    and jsx_attr_to_entity = attr =>
      (
        (
          switch (T.get_value(attr)) {
          | Class(name, value) => ("Class", name, value)

          | ID(name) => ("ID", name, None)

          | Property(name, value) => ("Property", name, value)
          }
        )
        |> Tuple.map_snd3(id_to_entity("Name"))
        |> (
          fun
          | (entity, name_child, Some(expr)) =>
            T.node_to_entity(
              ~children=[name_child, expr_to_entity(expr)],
              entity,
            )
          | (entity, name_child, None) =>
            T.node_to_entity(~children=[name_child], entity)
        )
      )(
        attr,
      )

    and stmt_to_entity = stmt =>
      (
        switch (T.get_value(stmt)) {
        | Variable(name, expr) =>
          T.node_to_entity(
            "Variable",
            ~children=[id_to_entity("Name", name), expr_to_entity(expr)],
          )

        | Expression(expr) =>
          T.node_to_entity("Expression", ~children=[expr_to_entity(expr)])
        }
      )(
        stmt,
      );
  };
};

module Raw =
  Make({
    type type_t = Type.Raw.t;

    type node_t('a) = Node.Raw.t('a);

    let get_value = Node.Raw.get_value;
    let get_range = Node.Raw.get_range;

    let node_to_entity = Common.Dump.raw_node_to_entity;
  });

module Analyzed =
  Make({
    type type_t = Type.Raw.t;

    type node_t('a) = Node.t('a, type_t);

    let get_value = Node.get_value;
    let get_type = Node.get_type;
    let get_range = Node.get_range;

    let node_to_entity = Common.Dump.analyzed_node_to_entity;
  });

include Make({
  type type_t = Type.t;

  type node_t('a) = Node.t('a, type_t);

  let get_value = Node.get_value;
  let get_type = Node.get_type;
  let get_range = Node.get_range;

  let node_to_entity = (~attributes=[], ~children=[], label, node) =>
    Common.Dump.node_to_entity(~attributes, ~children, label, node);
});

/**
 a declaration AST node
 */
type declaration_t = Node.t(raw_declaration_t, Type.t)
/**
 supported module declarations
 */
and raw_declaration_t =
  | Constant(expression_t)
  | Function(list(argument_t), expression_t);

/**
 an import AST node
 */
type import_t = Node.Raw.t(raw_import_t)
/**
 supported import types
 */
and raw_import_t =
  | MainImport(untyped_identifier_t)
  | NamedImport(untyped_identifier_t, option(untyped_identifier_t));

/**
 supported export types
 */
type export_t =
  | MainExport(untyped_identifier_t)
  | NamedExport(untyped_identifier_t);

/**
 module statement AST node
 */
type module_statement_t = Node.Raw.t(raw_module_statement_t)
/**
 supported top-level module statements
 */
and raw_module_statement_t =
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
let of_func = ((args, expr)) => Function(args, expr);

let of_import = ((namespace, main)) => Import(namespace, main);
let of_decl = ((name, x)) => Declaration(name, x);

module Dump = {
  include Dump;

  open Pretty.Formatters;

  let export_to_entity =
    fun
    | MainExport(id) => id_to_entity("MainExport", id)
    | NamedExport(id) => id_to_entity("NamedExport", id);

  let decl_to_entity: declaration_t => Entity.t =
    decl =>
      switch (Node.get_value(decl)) {
      | Constant(expr) =>
        node_to_entity(~children=[expr_to_entity(expr)], "Constant", decl)

      | Function(args, expr) =>
        node_to_entity(
          ~children=[
            Entity.create(
              ~children=
                args
                |> List.map(arg => {
                     let {name, default, type_} = Node.get_value(arg);
                     let children = ref([id_to_entity("Name", name)]);

                     switch (default) {
                     | Some(default) =>
                       children := [expr_to_entity(default), ...children^]
                     | None => ()
                     };

                     switch (type_) {
                     | Some(type_) =>
                       children :=
                         [node_to_entity("Type", type_), ...children^]
                     | None => ()
                     };

                     node_to_entity(~children=children^, "Argument", arg);
                   }),
              "Arguments",
            ),
            Entity.create(~children=[expr_to_entity(expr)], "Body"),
          ],
          "Function",
          decl,
        )
      };

  let import_to_entity = import =>
    switch (Node.Raw.get_value(import)) {
    | MainImport(name) =>
      raw_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "MainImport",
        import,
      )

    | NamedImport(name, Some((alias, alias_range))) =>
      raw_node_to_entity(
        ~children=[
          id_to_entity("Name", name),
          Entity.create(
            ~range=alias_range,
            ~attributes=[("value", Identifier.to_string(alias))],
            "Alias",
          ),
        ],
        "NamedImport",
        import,
      )

    | NamedImport(name, None) =>
      raw_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "NamedImport",
        import,
      )
    };

  let mod_stmt_to_entity = mod_stmt =>
    switch (Node.Raw.get_value(mod_stmt)) {
    | Import(namespace, imports) =>
      raw_node_to_entity(
        ~attributes=[("namespace", Namespace.to_string(namespace))],
        ~children=imports |> List.map(import_to_entity),
        "Import",
        mod_stmt,
      )

    | Declaration(name, decl) =>
      raw_node_to_entity(
        ~children=[export_to_entity(name), decl_to_entity(decl)],
        "Declaration",
        mod_stmt,
      )
    };

  let to_entity = program =>
    Entity.create(~children=program |> List.map(mod_stmt_to_entity), "AST");

  let pp: Fmt.t(program_t) = ppf => to_entity % Entity.pp(ppf);
};
