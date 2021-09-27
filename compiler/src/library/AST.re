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
   utilities for debugging an AST
   */
  module Debug = {
    open Pretty;

    let print_attr = ((name, value)) =>
      [string(" "), string(name), string("=\""), value, string("\"")]
      |> concat;

    let print_entity = (~attrs=[], ~children=[], ~range=?, name) =>
      [
        [
          string("<"),
          string(name),
          switch (range) {
          | Some(range) =>
            [string("@"), range |> ~@Range.pp |> string] |> concat
          | None => Nil
          },
          attrs |> List.map(print_attr) |> concat,
        ]
        |> concat,
        List.is_empty(children)
          ? string(" />")
          : [
              [string(">")] |> newline,
              children
              |> List.map(child => [child] |> newline)
              |> concat
              |> indent(2),
              [string("</"), string(name), string(">")] |> concat,
            ]
            |> concat,
      ]
      |> concat;

    let print_node = (~attrs=[], label, value, range) =>
      print_entity(~attrs, ~range, ~children=[value], label);

    let print_typed_node = (~attrs=[], label, value, type_, range) =>
      print_entity(
        ~attrs=[("type", type_ |> ~@Type.pp |> Pretty.string), ...attrs],
        ~range,
        ~children=[value],
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

  let print_node: (string, 'a => Pretty.t, node_t('a)) => Pretty.t;
};

/**
 contructor for AST modules
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

  module Debug = {
    open Pretty;

    include Debug;

    let print_node = T.print_node;
    let print_untyped_node = (label, print_value, x) =>
      Common.Debug.print_node(
        label,
        x |> Node.Raw.get_value |> print_value,
        Node.Raw.get_range(x),
      );

    let print_ns = ~@Namespace.pp % string;

    let print_id = ~@Identifier.pp % string;

    let print_binary_op =
      fun
      | LogicalAnd => "LogicalAnd"
      | LogicalOr => "LogicalOr"
      | Add => "Add"
      | Subtract => "Subtract"
      | Divide => "Divide"
      | Multiply => "Multiply"
      | LessOrEqual => "LessOrEqual"
      | LessThan => "LessThan"
      | GreaterOrEqual => "GreaterOrEqual"
      | GreaterThan => "GreaterThan"
      | Equal => "Equal"
      | Unequal => "Unequal"
      | Exponent => "Exponent";

    let print_unary_op =
      fun
      | Not => "Not"
      | Positive => "Positive"
      | Negative => "Negative";

    let rec print_expr =
      fun
      | Primitive(prim) => T.print_node("Primitive", print_prim, prim)
      | Identifier(id) => T.print_node("Identifier", print_id, id)
      | JSX(jsx) => T.print_node("JSX", print_jsx, jsx)
      | Group(group) => T.print_node("Group", print_expr, group)
      | BinaryOp(op, lhs, rhs) =>
        Debug.print_entity(
          ~children=[
            T.print_node("LeftHandSide", print_expr, lhs),
            T.print_node("RightHandSide", print_expr, rhs),
          ],
          print_binary_op(op),
        )
      | UnaryOp(op, expr) =>
        T.print_node(print_unary_op(op), print_expr, expr)
      | Closure(stmts) =>
        Debug.print_entity(
          ~children=stmts |> List.map(print_stmt),
          "Closure",
        )

    and print_prim =
      fun
      | Nil => string("nil")
      | Boolean(bool) => bool |> string_of_bool |> string
      | Number(num) => num |> print_num |> string
      | String(str) => str |> Fmt.str("\"%s\"") |> string

    and print_num =
      fun
      | Integer(int) => Int64.to_string(int)
      | Float(float, precision) => float |> Fmt.str("%.*f", precision)

    and print_jsx =
      fun
      | Tag(name, attrs, children) =>
        Debug.print_entity(
          ~attrs=[
            (
              "attrs",
              attrs
              |> List.map(attr =>
                   T.print_node("Attribute", print_jsx_attr, attr)
                 )
              |> concat,
            ),
            (
              "children",
              children
              |> List.map(child =>
                   T.print_node("Child", print_jsx_child, child)
                 )
              |> concat,
            ),
          ],
          ~children=[print_untyped_node("Name", print_id, name)],
          "Tag",
        )
      | Fragment(children) =>
        Debug.print_entity(
          ~children=
            children
            |> List.map(child =>
                 T.print_node("Child", print_jsx_child, child)
               ),
          "Fragment",
        )

    and print_jsx_child =
      fun
      | Node(jsx) => T.print_node("Node", print_jsx, jsx)
      | Text(text) => T.print_node("Text", Fmt.str("\"%s\"") % string, text)
      | InlineExpression(expr) =>
        T.print_node("InlineExpression", print_expr, expr)

    and print_jsx_attr = attr =>
      (
        switch (attr) {
        | Class(name, value) => (
            "Class",
            print_untyped_node("Name", print_id, name),
            value,
          )
        | ID(name) => (
            "ID",
            print_untyped_node("Name", print_id, name),
            None,
          )
        | Property(name, value) => (
            "Property",
            print_untyped_node("Name", print_id, name),
            value,
          )
        }
      )
      |> (
        fun
        | (entity, name, Some(expr)) =>
          Debug.print_entity(
            ~children=[name, T.print_node("Value", print_expr, expr)],
            entity,
          )
        | (entity, name, None) =>
          Debug.print_entity(~children=[name], entity)
      )

    and print_stmt = stmt =>
      T.print_node(
        "Statement",
        fun
        | Variable(name, expr) =>
          Debug.print_entity(
            ~children=[
              print_untyped_node("Name", print_id, name),
              T.print_node("Value", print_expr, expr),
            ],
            "Variable",
          )
        | Expression(expr) => T.print_node("Expression", print_expr, expr),
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

    let print_node = (label, print_value, x) =>
      Common.Debug.print_node(
        label,
        x |> get_value |> print_value,
        get_range(x),
      );
  });

include Make({
  type type_t = Type.t;

  type node_t('a) = Node.t('a);

  let get_value = Node.get_value;
  let get_type = Node.get_type;
  let get_range = Node.get_range;

  let print_node = (label, print_value, x) =>
    Common.Debug.print_typed_node(
      label,
      x |> get_value |> print_value,
      get_type(x),
      get_range(x),
    );
});

/**
 a declaration AST node
 */
type declaration_t = Node.t(raw_declaration_t)
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

/**
 utilities for debugging an AST
 */
module Debug = {
  open Pretty;

  include Debug;

  let print_decl = ((name, decl)) =>
    switch (Node.get_value(decl)) {
    | Constant(expr) =>
      print_entity(
        ~range=Node.get_range(decl),
        ~children=[
          print_entity(
            ~range=
              switch (name) {
              | MainExport(x)
              | NamedExport(x) => Node.Raw.get_range(x)
              },
            ~children=[
              switch (name) {
              | MainExport(id) =>
                [string("(main) "), id |> Node.Raw.get_value |> print_id]
                |> concat
              | NamedExport(id) => id |> Node.Raw.get_value |> print_id
              },
            ],
            "Name",
          ),
          print_node("Value", print_expr, expr),
        ],
        "Constant",
      )
    | Function(args, expr) =>
      print_entity(
        ~children=[
          print_entity(
            ~children=
              args
              |> List.map((({name, default}, type_, _)) =>
                   [
                     print_untyped_node("Name", print_id, name),
                     print_entity(
                       ~children=[type_ |> ~@Type.pp |> string],
                       "Type",
                     ),
                     ...switch (default) {
                        | Some(default) => [
                            print_node("Default", print_expr, default),
                          ]
                        | None => []
                        },
                   ]
                 )
              |> List.flatten,
            "Attributes",
          ),
          print_entity(
            ~children=[print_node("Value", print_expr, expr)],
            "Body",
          ),
        ],
        "Function",
      )
    };

  let print_mod_stmt =
    (
      fun
      | (Import(namespace, imports), range) => [
          print_entity(
            ~range,
            ~attrs=[
              ("namespace", print_ns(namespace)),
              ("main", print_ns(namespace)),
            ],
            ~children=
              imports
              |> List.map(
                   Node.Raw.get_value
                   % (
                     fun
                     | MainImport(name) =>
                       print_entity(
                         "Main",
                         ~children=[
                           print_untyped_node("Name", print_id, name),
                         ],
                       )
                     | NamedImport(name, Some((label, label_range))) =>
                       print_entity(
                         "Named",
                         ~children=[
                           print_untyped_node("Name", print_id, name),
                           print_entity(
                             ~range=label_range,
                             ~children=[print_id(label)],
                             "As",
                           ),
                         ],
                       )
                     | NamedImport(name, None) =>
                       print_entity(
                         "Named",
                         ~children=[
                           print_untyped_node("Name", print_id, name),
                         ],
                       )
                   ),
                 ),
            "Import",
          ),
        ]
      | (Declaration(name, decl), range) => [
          print_entity(
            ~range,
            ~children=[(name, decl) |> print_decl],
            "Declaration",
          ),
        ]
    )
    % newline;

  let print_ast = (program: program_t): Pretty.t =>
    program |> List.map(print_mod_stmt) |> concat;
};

let to_string = (program: program_t): string =>
  program |> Debug.print_ast |> Pretty.to_string;
