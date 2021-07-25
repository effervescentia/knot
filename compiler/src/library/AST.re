/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Infix;
open Reference;

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

type lexeme_t('a) = ('a, Cursor.t);

type identifier_t = lexeme_t(Identifier.t);

module type ASTParams = {
  type type_t;

  let type_to_string: type_t => string;
};

module Make = (T: ASTParams) => {
  type type_t = T.type_t;

  type typed_lexeme_t('a) = ('a, type_t, Cursor.t);

  type primitive_t = typed_lexeme_t(raw_primitive_t)
  and raw_primitive_t =
    | Nil
    | Boolean(bool)
    | Number(number_t)
    | String(string);

  type jsx_t = lexeme_t(raw_jsx_t)
  and raw_jsx_t =
    | Tag(identifier_t, list(jsx_attribute_t), list(jsx_child_t))
    | Fragment(list(jsx_child_t))
  and jsx_child_t = lexeme_t(raw_jsx_child_t)
  and raw_jsx_child_t =
    | Text(lexeme_t(string))
    | Node(jsx_t)
    | InlineExpression(expression_t)
  and jsx_attribute_t = lexeme_t(raw_jsx_attribute_t)
  and raw_jsx_attribute_t =
    | ID(identifier_t)
    | Class(identifier_t, option(expression_t))
    | Property(identifier_t, option(expression_t))
  and expression_t = typed_lexeme_t(raw_expression_t)
  and raw_expression_t =
    | Primitive(primitive_t)
    | Identifier(identifier_t)
    | JSX(jsx_t)
    | Group(expression_t)
    | BinaryOp(binary_operator_t, expression_t, expression_t)
    | UnaryOp(unary_operator_t, expression_t)
    | Closure(list(statement_t))
  and statement_t =
    | Variable(identifier_t, expression_t)
    | Expression(expression_t);

  type argument_t = {
    name: identifier_t,
    default: option(expression_t),
    /* type_: option(lexeme_t(Type.t)), */
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

    let _print_attr = ((name, value)) =>
      [string(" "), string(name), string("=\""), value, string("\"")]
      |> concat;

    let _print_entity = (~attrs=[], ~children=[], ~cursor=?, name) =>
      [
        [
          string("<"),
          string(name),
          switch (cursor) {
          | Some(cursor) =>
            [string("@"), cursor |> Cursor.to_string |> string] |> concat
          | None => Nil
          },
          attrs |> List.map(_print_attr) |> concat,
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
      _print_entity(~attrs, ~cursor, ~children=[value], name);

    let print_typed_lexeme = (name, value, type_, cursor) =>
      print_lexeme(
        ~attrs=[("type", type_ |> T.type_to_string |> string)],
        name,
        value,
        cursor,
      );

    let rec print_expr =
      fun
      | Primitive((prim, type_, cursor)) =>
        print_typed_lexeme("Primitive", print_prim(prim), type_, cursor)
      | Identifier((name, cursor)) =>
        print_lexeme("Identifier", print_id(name), cursor)
      | JSX((jsx, cursor)) => print_lexeme("JSX", print_jsx(jsx), cursor)
      | Group((expr, type_, cursor)) =>
        print_typed_lexeme("Group", print_expr(expr), type_, cursor)
      | BinaryOp(
          op,
          (l_value, l_type, l_cursor),
          (r_value, r_type, r_cursor),
        ) =>
        _print_entity(
          ~children=[
            print_typed_lexeme(
              "LeftHandSide",
              print_expr(l_value),
              l_type,
              l_cursor,
            ),
            print_typed_lexeme(
              "RightHandSide",
              print_expr(r_value),
              r_type,
              r_cursor,
            ),
          ],
          op |> print_binary_op,
        )
      | UnaryOp(op, (expr, type_, cursor)) =>
        print_typed_lexeme(
          print_unary_op(op),
          print_expr(expr),
          type_,
          cursor,
        )
      | Closure(stmts) =>
        _print_entity(~children=stmts |> List.map(print_stmt), "Closure")

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
      | Tag((name, cursor), attrs, children) =>
        _print_entity(
          ~attrs=[
            (
              "attrs",
              attrs
              |> List.map(((attr, cursor)) =>
                   print_lexeme("Attribute", print_jsx_attr(attr), cursor)
                 )
              |> concat,
            ),
            (
              "children",
              children
              |> List.map(((child, cursor)) =>
                   print_lexeme("Child", print_jsx_child(child), cursor)
                 )
              |> concat,
            ),
          ],
          ~children=[print_lexeme("Name", print_id(name), cursor)],
          "Tag",
        )
      | Fragment(children) =>
        _print_entity(
          ~children=
            children
            |> List.map(((child, cursor)) =>
                 print_lexeme("Child", print_jsx_child(child), cursor)
               ),
          "Fragment",
        )

    and print_jsx_child =
      fun
      | Node((jsx, cursor)) => print_lexeme("Node", print_jsx(jsx), cursor)
      | Text((text, cursor)) =>
        print_lexeme("Text", text |> Print.fmt("\"%s\"") |> string, cursor)
      | InlineExpression((expr, type_, cursor)) =>
        print_typed_lexeme(
          "InlineExpression",
          expr |> print_expr,
          type_,
          cursor,
        )

    and print_jsx_attr = attr =>
      (
        switch (attr) {
        | Class((name, cursor), value) => (
            "Class",
            print_lexeme("Name", print_id(name), cursor),
            value,
          )
        | ID((name, cursor)) => (
            "ID",
            print_lexeme("Name", print_id(name), cursor),
            None,
          )
        | Property((name, cursor), value) => (
            "Property",
            print_lexeme("Name", print_id(name), cursor),
            value,
          )
        }
      )
      |> (
        fun
        | (entity, name, Some((expr, type_, cursor))) =>
          _print_entity(
            ~children=[
              name,
              print_typed_lexeme("Value", print_expr(expr), type_, cursor),
            ],
            entity,
          )
        | (entity, name, None) => _print_entity(~children=[name], entity)
      )

    and print_stmt = stmt =>
      switch (stmt) {
      | Variable((name, name_cursor), (expr, type_, cursor)) =>
        _print_entity(
          ~children=[
            print_lexeme("Name", name |> print_id, name_cursor),
            print_typed_lexeme("Value", print_expr(expr), type_, cursor),
          ],
          "Variable",
        )
      | Expression((expr, type_, cursor)) =>
        print_typed_lexeme("Expression", print_expr(expr), type_, cursor)
      };
  };
};

module Raw =
  Make({
    type type_t = Type2.Raw.t;

    let type_to_string = Type2.Raw.to_string;
  });

include Make({
  type type_t = Type2.t;

  let type_to_string = Type2.to_raw % Type2.Raw.to_string;
});

type declaration_t =
  | Constant(expression_t)
  | Function(list((argument_t, type_t)), expression_t);

type import_t =
  | MainImport(identifier_t)
  | NamedImport(identifier_t, option(identifier_t));

type export_t =
  | MainExport(identifier_t)
  | NamedExport(identifier_t);

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
    | Constant((expr, type_, cursor)) =>
      _print_entity(
        ~children=[
          print_lexeme(
            "Name",
            switch (name) {
            | MainExport((id, _)) =>
              [string("(main) "), print_id(id)] |> concat
            | NamedExport((id, _)) => print_id(id)
            },
            switch (name) {
            | MainExport((_, name_cursor))
            | NamedExport((_, name_cursor)) => name_cursor
            },
          ),
          print_typed_lexeme("Value", print_expr(expr), type_, cursor),
        ],
        "Constant",
      )
    | Function(args, expr) =>
      _print_entity(
        ~children=[
          _print_entity(
            ~children=
              args
              |> List.map((({name, default}, type_)) =>
                   [
                     print_lexeme("Name", name |> fst |> print_id, snd(name)),
                     _print_entity(
                       ~children=[name |> fst |> print_id],
                       "Type",
                     ),
                     ...switch (default) {
                        | Some((default, type_, cursor)) => [
                            print_typed_lexeme(
                              "Default",
                              print_expr(default),
                              type_,
                              cursor,
                            ),
                          ]
                        | None => []
                        },
                   ]
                 )
              |> List.flatten,
            "Attributes",
          ),
          _print_entity(
            ~children=[
              print_typed_lexeme(
                "Value",
                expr |> Tuple.fst3 |> print_expr,
                Tuple.snd3(expr),
                Tuple.thd3(expr),
              ),
            ],
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
          _print_entity(
            ~attrs=[
              ("namespace", print_ns(namespace)),
              ("main", print_ns(namespace)),
            ],
            ~children=
              imports
              |> List.map(
                   fun
                   | MainImport((name, cursor)) =>
                     _print_entity(
                       "Main",
                       ~children=[
                         print_lexeme("Name", print_id(name), cursor),
                       ],
                     )
                   | NamedImport(
                       (name, name_cursor),
                       Some((label, label_cursor)),
                     ) =>
                     _print_entity(
                       "Named",
                       ~children=[
                         print_lexeme("Name", print_id(name), name_cursor),
                         print_lexeme("As", print_id(label), label_cursor),
                       ],
                     )
                   | NamedImport((name, cursor), None) =>
                     _print_entity(
                       "Named",
                       ~children=[
                         print_lexeme("Name", print_id(name), cursor),
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
