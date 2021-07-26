/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Infix;
open Reference;

module Common = {
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

  type unary_operator_t =
    | Not
    | Positive
    | Negative;

  type number_t =
    | Integer(Int64.t)
    | Float(float, int);

  module Debug = {
    open Pretty;

    let print_attr = ((name, value)) =>
      [string(" "), string(name), string("=\""), value, string("\"")]
      |> concat;

    let print_entity = (~attrs=[], ~children=[], ~cursor=?, name) =>
      [
        [
          string("<"),
          string(name),
          switch (cursor) {
          | Some(cursor) =>
            [string("@"), cursor |> Cursor.to_string |> string] |> concat
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

    let print_lexeme = (~attrs=[], name, value, cursor) =>
      print_entity(~attrs, ~cursor, ~children=[value], name);
  };
};

module type ASTParams = {
  type type_t;

  type lexeme_t('a);

  let get_value: lexeme_t('a) => 'a;
  let get_cursor: lexeme_t('a) => Cursor.t;

  let print_lexeme: (string, 'a => Pretty.t, lexeme_t('a)) => Pretty.t;
};

module Make = (T: ASTParams) => {
  include Common;

  type type_t = T.type_t;

  type identifier_t = T.lexeme_t(Identifier.t);
  type untyped_id_t = Block.t(Identifier.t);

  type primitive_t = T.lexeme_t(raw_primitive_t)
  and raw_primitive_t =
    | Nil
    | Boolean(bool)
    | Number(number_t)
    | String(string);

  type jsx_t = T.lexeme_t(raw_jsx_t)
  and raw_jsx_t =
    | Tag(untyped_id_t, list(jsx_attribute_t), list(jsx_child_t))
    | Fragment(list(jsx_child_t))
  and jsx_child_t = T.lexeme_t(raw_jsx_child_t)
  and raw_jsx_child_t =
    | Text(T.lexeme_t(string))
    | Node(jsx_t)
    | InlineExpression(expression_t)
  and jsx_attribute_t = T.lexeme_t(raw_jsx_attribute_t)
  and raw_jsx_attribute_t =
    | ID(untyped_id_t)
    | Class(untyped_id_t, option(expression_t))
    | Property(untyped_id_t, option(expression_t))
  and expression_t = T.lexeme_t(raw_expression_t)
  and raw_expression_t =
    | Primitive(primitive_t)
    | Identifier(identifier_t)
    | JSX(jsx_t)
    | Group(expression_t)
    | BinaryOp(binary_operator_t, expression_t, expression_t)
    | UnaryOp(unary_operator_t, expression_t)
    | Closure(list(statement_t))
  and statement_t =
    | Variable(untyped_id_t, expression_t)
    | Expression(expression_t);

  type argument_t = T.lexeme_t(raw_argument_t)
  and raw_argument_t = {
    name: untyped_id_t,
    default: option(expression_t),
    type_: option(T.lexeme_t(Type.t)),
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

    let print_lexeme = T.print_lexeme;
    let print_untyped_lexeme = (label, print_value, x) =>
      Common.Debug.print_lexeme(
        label,
        x |> Block.value |> print_value,
        Block.cursor(x),
      );

    let print_ns = Namespace.to_string % string;

    let print_id = Identifier.to_string % string;

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
      | Primitive(prim) => T.print_lexeme("Primitive", print_prim, prim)
      | Identifier(id) => T.print_lexeme("Identifier", print_id, id)
      | JSX(jsx) => T.print_lexeme("JSX", print_jsx, jsx)
      | Group(group) => T.print_lexeme("Group", print_expr, group)
      | BinaryOp(op, lhs, rhs) =>
        Debug.print_entity(
          ~children=[
            T.print_lexeme("LeftHandSide", print_expr, lhs),
            T.print_lexeme("RightHandSide", print_expr, rhs),
          ],
          print_binary_op(op),
        )
      | UnaryOp(op, expr) =>
        T.print_lexeme(print_unary_op(op), print_expr, expr)
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
      | String(str) => str |> Print.fmt("\"%s\"") |> string

    and print_num =
      fun
      | Integer(int) => Int64.to_string(int)
      | Float(float, precision) => float |> Print.fmt("%.*f", precision)

    and print_jsx =
      fun
      | Tag(name, attrs, children) =>
        Debug.print_entity(
          ~attrs=[
            (
              "attrs",
              attrs
              |> List.map(attr =>
                   T.print_lexeme("Attribute", print_jsx_attr, attr)
                 )
              |> concat,
            ),
            (
              "children",
              children
              |> List.map(child =>
                   T.print_lexeme("Child", print_jsx_child, child)
                 )
              |> concat,
            ),
          ],
          ~children=[print_untyped_lexeme("Name", print_id, name)],
          "Tag",
        )
      | Fragment(children) =>
        Debug.print_entity(
          ~children=
            children
            |> List.map(child =>
                 T.print_lexeme("Child", print_jsx_child, child)
               ),
          "Fragment",
        )

    and print_jsx_child =
      fun
      | Node(jsx) => T.print_lexeme("Node", print_jsx, jsx)
      | Text(text) =>
        T.print_lexeme("Text", Print.fmt("\"%s\"") % string, text)
      | InlineExpression(expr) =>
        T.print_lexeme("InlineExpression", print_expr, expr)

    and print_jsx_attr = attr =>
      (
        switch (attr) {
        | Class(name, value) => (
            "Class",
            print_untyped_lexeme("Name", print_id, name),
            value,
          )
        | ID(name) => (
            "ID",
            print_untyped_lexeme("Name", print_id, name),
            None,
          )
        | Property(name, value) => (
            "Property",
            print_untyped_lexeme("Name", print_id, name),
            value,
          )
        }
      )
      |> (
        fun
        | (entity, name, Some(expr)) =>
          Debug.print_entity(
            ~children=[name, T.print_lexeme("Value", print_expr, expr)],
            entity,
          )
        | (entity, name, None) =>
          Debug.print_entity(~children=[name], entity)
      )

    and print_stmt = stmt =>
      switch (stmt) {
      | Variable(name, expr) =>
        Debug.print_entity(
          ~children=[
            print_untyped_lexeme("Name", print_id, name),
            T.print_lexeme("Value", print_expr, expr),
          ],
          "Variable",
        )
      | Expression(expr) => T.print_lexeme("Expression", print_expr, expr)
      };
  };
};

module Raw =
  Make({
    type type_t = Type.Raw.t;

    type lexeme_t('a) = Block.t('a);

    let get_value = Block.value;
    let get_cursor = Block.cursor;

    let print_lexeme = (label, print_value, x) =>
      Common.Debug.print_lexeme(
        label,
        x |> get_value |> print_value,
        get_cursor(x),
      );
  });

include Make({
  type type_t = Type.t;

  type lexeme_t('a) = ('a, Type.t, Cursor.t);

  let get_value = Tuple.fst3;
  let get_type = Tuple.snd3;
  let get_cursor = Tuple.thd3;

  let print_lexeme = (label, print_value, x) =>
    Common.Debug.print_lexeme(
      ~attrs=[("type", x |> get_type |> Type.to_string |> Pretty.string)],
      label,
      x |> get_value |> print_value,
      get_cursor(x),
    );
});

type declaration_t =
  | Constant(expression_t)
  | Function(list(argument_t), expression_t);

type import_t =
  | MainImport(untyped_id_t)
  | NamedImport(untyped_id_t, option(untyped_id_t));

type export_t =
  | MainExport(untyped_id_t)
  | NamedExport(untyped_id_t);

type module_statement_t =
  | Import(Namespace.t, list(import_t))
  | Declaration(export_t, declaration_t);

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
let of_const = x => Constant(x);
let of_func = ((args, expr)) => Function(args, expr);

module Debug = {
  open Pretty;

  include Debug;

  let print_decl = ((name, decl)) =>
    switch (decl) {
    | Constant(expr) =>
      print_entity(
        ~children=[
          print_entity(
            ~cursor=
              switch (name) {
              | MainExport((_, name_cursor))
              | NamedExport((_, name_cursor)) => name_cursor
              },
            ~children=[
              switch (name) {
              | MainExport((id, _)) =>
                [string("(main) "), print_id(id)] |> concat
              | NamedExport((id, _)) => print_id(id)
              },
            ],
            "Name",
          ),
          print_lexeme("Value", print_expr, expr),
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
                     print_untyped_lexeme("Name", print_id, name),
                     print_entity(
                       ~children=[type_ |> Type.to_string |> string],
                       "Type",
                     ),
                     ...switch (default) {
                        | Some(default) => [
                            print_lexeme("Default", print_expr, default),
                          ]
                        | None => []
                        },
                   ]
                 )
              |> List.flatten,
            "Attributes",
          ),
          print_entity(
            ~children=[print_lexeme("Value", print_expr, expr)],
            "Body",
          ),
        ],
        "Function",
      )
    };

  let print_mod_stmt =
    (
      fun
      | Import(namespace, imports) => [
          print_entity(
            ~attrs=[
              ("namespace", print_ns(namespace)),
              ("main", print_ns(namespace)),
            ],
            ~children=
              imports
              |> List.map(
                   fun
                   | MainImport(name) =>
                     print_entity(
                       "Main",
                       ~children=[
                         print_untyped_lexeme("Name", print_id, name),
                       ],
                     )
                   | NamedImport(name, Some((label, label_cursor))) =>
                     print_entity(
                       "Named",
                       ~children=[
                         print_untyped_lexeme("Name", print_id, name),
                         print_entity(
                           ~cursor=label_cursor,
                           ~children=[print_id(label)],
                           "As",
                         ),
                       ],
                     )
                   | NamedImport(name, None) =>
                     print_entity(
                       "Named",
                       ~children=[
                         print_untyped_lexeme("Name", print_id, name),
                       ],
                     ),
                 ),
            "Import",
          ),
        ]
      | Declaration(name, decl) => [(name, decl) |> print_decl]
    )
    % newline;

  let print_ast = (program: program_t): string =>
    program |> List.map(print_mod_stmt) |> concat |> to_string;
};
