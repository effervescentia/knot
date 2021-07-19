/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Infix;
open Reference;
open Cow;

module type ASTParams = {
  type type_t;

  let print_type: type_t => string;
};

module GlobalDebug = Debug;

type lexeme_t('a) = ('a, Cursor.t);

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

type identifier_t = lexeme_t(Identifier.t);

type raw_primitive_t =
  | Nil
  | Boolean(bool)
  | Number(number_t)
  | String(string);

module Make = (T: ASTParams) => {
  type typed_lexeme_t('a) = ('a, T.type_t, Cursor.t);

  type primitive_t = typed_lexeme_t(raw_primitive_t);

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

  type declaration_t =
    | Constant(expression_t)
    | Type(lexeme_t(T.type_t))
    | Function(list((argument_t, T.type_t)), expression_t);

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

  module Util = {
    let internal = namespace => Namespace.Internal(namespace);
    let external_ = namespace => Namespace.External(namespace);

    let public = name => Identifier.Public(name);
    let private = name => Identifier.Private(name);

    let main_import = x => MainImport(x);
    let named_import = ((x, y)) => NamedImport(x, y);

    let main_export = x => MainExport(x);
    let named_export = x => NamedExport(x);

    let import = ((namespace, main)) => Import(namespace, main);
    let decl = ((name, x)) => Declaration(name, x);
    let const = x => Constant(x);
    let type_ = x => Type(x);
    let func = ((args, expr)) => Function(args, expr);
    let var = ((name, x)) => Variable(name, x);
    let expr = x => Expression(x);
    let id = x => Identifier(x);
    let group = x => Group(x);
    let closure = xs => Closure(xs);

    let not_op = x => UnaryOp(Not, x);
    let neg_op = x => UnaryOp(Negative, x);
    let pos_op = x => UnaryOp(Positive, x);

    let and_op = ((l, r)) => BinaryOp(LogicalAnd, l, r);
    let or_op = ((l, r)) => BinaryOp(LogicalOr, l, r);

    let mult_op = ((l, r)) => BinaryOp(Multiply, l, r);
    let div_op = ((l, r)) => BinaryOp(Divide, l, r);
    let add_op = ((l, r)) => BinaryOp(Add, l, r);
    let sub_op = ((l, r)) => BinaryOp(Subtract, l, r);

    let lt_op = ((l, r)) => BinaryOp(LessThan, l, r);
    let lte_op = ((l, r)) => BinaryOp(LessOrEqual, l, r);
    let gt_op = ((l, r)) => BinaryOp(GreaterThan, l, r);
    let gte_op = ((l, r)) => BinaryOp(GreaterOrEqual, l, r);

    let eq_op = ((l, r)) => BinaryOp(Equal, l, r);
    let ineq_op = ((l, r)) => BinaryOp(Unequal, l, r);

    let expo_op = ((l, r)) => BinaryOp(Exponent, l, r);

    let jsx = x => JSX(x);
    let frag = xs => Fragment(xs);
    let tag = ((name, attrs, children)) => Tag(name, attrs, children);
    let prop = ((name, value)) => Property(name, value);
    let jsx_class = ((name, value)) => Class(name, value);
    let jsx_id = x => ID(x);
    let text = x => Text(x);
    let node = x => Node(x);
    let inline_expr = x => InlineExpression(x);

    let prim = x => Primitive(x);
    let bool = x => Boolean(x);
    let int = x => Integer(x);
    let float = ((x, precision)) => Float(x, precision);
    let string = x => String(x);
    let num = x => Number(x);
    let nil = Nil;
  };

  module Debug = {
    let print_lexeme = (~attrs=[], name, value, cursor) =>
      Xml.tag(
        name,
        ~attrs=[("cursor", cursor |> Cursor.to_string), ...attrs],
        value,
      );

    let print_typed_lexeme = (name, value, type_, cursor) =>
      Xml.tag(
        name,
        ~attrs=[
          ("type", type_ |> T.print_type),
          ("cursor", cursor |> Cursor.to_string),
        ],
        value,
      );

    let print_id = Identifier.to_string % Xml.string;

    let print_binary_op =
      fun
      | LogicalAnd => "Logical_And"
      | LogicalOr => "Logical_Or"
      | Add => "Add"
      | Subtract => "Subtract"
      | Divide => "Divide"
      | Multiply => "Multiply"
      | LessOrEqual => "Less_Or_Equal"
      | LessThan => "Less_Than"
      | GreaterOrEqual => "Greater_Or_Equal"
      | GreaterThan => "Greater_Than"
      | Equal => "Equal"
      | Unequal => "Unequal"
      | Exponent => "Exponent";

    let print_unary_op =
      fun
      | Not => "Not"
      | Positive => "Positive"
      | Negative => "Negative";

    let print_num =
      fun
      | Integer(int) => int |> Int64.to_string
      | Float(float, precision) => float |> Print.fmt("%.*f", precision);

    let print_prim =
      fun
      | Nil => "nil"
      | Boolean(bool) => bool |> string_of_bool
      | Number(num) => num |> print_num
      | String(str) => str |> Print.fmt("\"%s\"");

    let rec print_expr =
      fun
      | Primitive((prim, type_, cursor)) =>
        print_typed_lexeme(
          "Primitive",
          prim |> print_prim |> Xml.string,
          type_,
          cursor,
        )
      | Identifier((name, cursor)) =>
        print_lexeme("Identifier", name |> print_id, cursor)
      | JSX((jsx, cursor)) => print_lexeme("JSX", jsx |> print_jsx, cursor)
      | Group((expr, type_, cursor)) =>
        print_typed_lexeme("Group", expr |> print_expr, type_, cursor)
      | BinaryOp(
          op,
          (l_value, l_type, l_cursor),
          (r_value, r_type, r_cursor),
        ) =>
        Xml.tag(
          op |> print_binary_op,
          [
            print_typed_lexeme(
              "Left_Hand_Side",
              l_value |> print_expr,
              l_type,
              l_cursor,
            ),
            print_typed_lexeme(
              "Right_Hand_Side",
              r_value |> print_expr,
              r_type,
              r_cursor,
            ),
          ]
          |> Xml.list,
        )
      | UnaryOp(op, (expr, type_, cursor)) =>
        print_typed_lexeme(
          op |> print_unary_op,
          expr |> print_expr,
          type_,
          cursor,
        )
      | Closure(stmts) =>
        Xml.tag("Closure", stmts |> List.map(print_stmt) |> Xml.list)

    and print_stmt = stmt =>
      switch (stmt) {
      | Variable((name, name_cursor), (expr, type_, cursor)) =>
        Xml.tag(
          "Variable",
          [
            print_lexeme("Name", name |> print_id, name_cursor),
            print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
          ]
          |> Xml.list,
        )
      | Expression((expr, type_, cursor)) =>
        print_typed_lexeme("Expression", expr |> print_expr, type_, cursor)
      }

    and print_jsx =
      fun
      | Tag((name, cursor), attrs, children) =>
        Xml.tag(
          "Tag",
          [
            print_lexeme("Name", name |> print_id, cursor),
            Xml.tag(
              "Attributes",
              attrs
              |> List.map(((attr, cursor)) =>
                   print_lexeme("Attribute", attr |> print_jsx_attr, cursor)
                 )
              |> Xml.list,
            ),
            Xml.tag(
              "Children",
              children
              |> List.map(((child, cursor)) =>
                   print_lexeme("Child", child |> print_jsx_child, cursor)
                 )
              |> Xml.list,
            ),
          ]
          |> Xml.list,
        )
      | Fragment(children) =>
        Xml.tag(
          "Fragment",
          children
          |> List.map(((child, cursor)) =>
               print_lexeme("Child", child |> print_jsx_child, cursor)
             )
          |> Xml.list,
        )

    and print_jsx_child =
      fun
      | Node((jsx, cursor)) => print_lexeme("Node", jsx |> print_jsx, cursor)
      | Text((text, cursor)) =>
        print_lexeme(
          "Text",
          text |> Print.fmt("\"%s\"") |> Xml.string,
          cursor,
        )
      | InlineExpression((expr, type_, cursor)) =>
        print_typed_lexeme(
          "Inline_Expression",
          expr |> print_expr,
          type_,
          cursor,
        )

    and print_jsx_attr = attr =>
      (
        switch (attr) {
        | Class((name, cursor), value) => (
            "Class",
            print_lexeme("Name", name |> print_id, cursor),
            value,
          )
        | ID((name, cursor)) => (
            "ID",
            print_lexeme("Name", name |> print_id, cursor),
            None,
          )
        | Property((name, cursor), value) => (
            "Property",
            print_lexeme("Name", name |> print_id, cursor),
            value,
          )
        }
      )
      |> (
        fun
        | (entity, name, Some((expr, type_, cursor))) =>
          Xml.tag(
            entity,
            [
              name,
              print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
            ]
            |> Xml.list,
          )
        | (entity, name, None) => Xml.tag(entity, name)
      );

    let print_decl = ((name, decl)) =>
      switch (decl) {
      | Constant((expr, type_, cursor)) =>
        Xml.tag(
          "Constant",
          [
            print_lexeme(
              "Name",
              switch (name) {
              | MainExport((id, _)) =>
                id
                |> Identifier.to_string
                |> Print.fmt("(main) %s")
                |> Xml.string
              | NamedExport((id, _)) => id |> print_id
              },
              switch (name) {
              | MainExport((_, name_cursor))
              | NamedExport((_, name_cursor)) => name_cursor
              },
            ),
            print_typed_lexeme("Value", expr |> print_expr, type_, cursor),
          ]
          |> Xml.list,
        )
      | Type((type_, cursor)) =>
        Xml.tag(
          "Type",
          [
            print_lexeme(
              "Name",
              switch (name) {
              | MainExport((id, _)) =>
                id
                |> Identifier.to_string
                |> Print.fmt("(main) %s")
                |> Xml.string
              | NamedExport((id, _)) => id |> print_id
              },
              switch (name) {
              | MainExport((_, name_cursor))
              | NamedExport((_, name_cursor)) => name_cursor
              },
            ),
            print_lexeme(
              "Value",
              type_ |> T.print_type |> Xml.string,
              cursor,
            ),
          ]
          |> Xml.list,
        )
      | Function(args, expr) =>
        Xml.tag(
          "Function",
          [
            Xml.tag(
              "Attributes",
              args
              |> List.map((({name, default}, type_)) =>
                   [
                     print_lexeme(
                       "Name",
                       name |> fst |> print_id,
                       name |> snd,
                     ),
                     Xml.tag("Type", name |> fst |> print_id),
                     ...switch (default) {
                        | Some((default, type_, cursor)) => [
                            print_typed_lexeme(
                              "Default",
                              default |> print_expr,
                              type_,
                              cursor,
                            ),
                          ]
                        | None => []
                        },
                   ]
                 )
              |> List.flatten
              |> Xml.list,
            ),
            Xml.tag(
              "Body",
              print_typed_lexeme(
                "Value",
                expr |> Tuple.fst3 |> print_expr,
                expr |> Tuple.snd3,
                expr |> Tuple.thd3,
              ),
            ),
          ]
          |> Xml.list,
        )
      };

    let print_mod_stmt =
      fun
      | Import(namespace, imports) => [
          Xml.tag(
            "Import",
            ~attrs=[
              ("namespace", namespace |> Namespace.to_string),
              ("main", namespace |> Namespace.to_string),
            ],
            imports
            |> List.map(
                 fun
                 | MainImport((name, cursor)) =>
                   Xml.tag(
                     "Main",
                     print_lexeme("Name", name |> print_id, cursor),
                   )
                 | NamedImport(
                     (name, name_cursor),
                     Some((label, label_cursor)),
                   ) =>
                   Xml.tag(
                     "Named",
                     [
                       print_lexeme("Name", name |> print_id, name_cursor),
                       print_lexeme("As", label |> print_id, label_cursor),
                     ]
                     |> Xml.list,
                   )
                 | NamedImport((name, cursor), None) =>
                   Xml.tag(
                     "Named",
                     print_lexeme("Name", name |> print_id, cursor),
                   ),
               )
            |> Xml.list,
          ),
        ]
      | Declaration(name, decl) => [(name, decl) |> print_decl];

    let print_ast = (program: program_t): string =>
      program
      |> List.map(print_mod_stmt)
      |> List.flatten
      |> Xml.list
      |> Xml.to_string;
  };
};

module Raw =
  Make({
    type type_t = Type.t;

    let print_type = Type.to_string;
  });

include Raw;

module Final =
  Make({
    open Constants;

    type type_t = Type2.t;

    let rec print_type =
      Type2.(
        fun
        | Nil => Keyword.nil
        | Boolean => Keyword.bool
        | Integer => Keyword.int
        | Float => Keyword.float
        | String => Keyword.string
        | Element => Keyword.element
        | Iterable(t) => Keyword.element
        | Abstract(abstr) =>
          switch (abstr) {
          | Unknown => "unknown"
          | Numeric => Keyword.number
          }
        /* TODO: finish this */
        | pattern => "pattern"
      );
  });
