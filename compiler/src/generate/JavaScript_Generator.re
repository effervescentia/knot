/**
 Utilities for converting module AST into JavaScript code.
 */
open Kore;
open ModuleAliases;

module Identifier = Reference.Identifier;

let __util_lib = "$knot";
let __runtime_namespace = "@knot/runtime";
let __class_name_prop = "className";
let __arguments_object = "arguments";
let __id_prop = "id";
let __main_export = "main";
let __view_props = "$props$";

let _knot_util = (util, property) =>
  JavaScript_AST.DotAccess(
    DotAccess(Identifier(__util_lib), util),
    property,
  );
let _jsx_util = _knot_util("jsx");
let _platform_util = _knot_util("platform");

let __knot_arg = _platform_util("arg");
let __knot_prop = _platform_util("prop");
let __jsx_create_tag = _jsx_util("createTag");
let __jsx_create_fragment = _jsx_util("createFragment");

let gen_number = x =>
  JavaScript_AST.Number(
    switch (x) {
    | A.Integer(value) => Int64.to_string(value)
    | A.Float(value, precision) => value |> Fmt.str("%.*g", precision)
    },
  );

let rec gen_expression =
  fun
  | A.Primitive(Boolean(x)) => JavaScript_AST.Boolean(x)
  | A.Primitive(Number(x)) => gen_number(x)
  | A.Primitive(String(x)) => JavaScript_AST.String(x)
  | A.Primitive(Nil) => JavaScript_AST.Null
  | A.Identifier(value) =>
    JavaScript_AST.Identifier(value |> ~@Identifier.pp)
  | A.Group(value) =>
    JavaScript_AST.Group(value |> N.get_value |> gen_expression)

  | A.Closure([]) => JavaScript_AST.(Null)
  | A.Closure(values) => {
      let rec loop = (
        fun
        | [] => []
        | [x] => x |> N.get_value |> gen_statement(~is_last=true)
        | [x, ...xs] => (x |> N.get_value |> gen_statement) @ loop(xs)
      );

      values |> loop |> JavaScript_AST.iife;
    }

  | A.UnaryOp(op, value) => value |> gen_unary_op(op)
  | A.BinaryOp(op, lhs, rhs) => gen_binary_op(op, lhs, rhs)
  | A.JSX(value) => gen_jsx(value)

and gen_statement = (~is_last=false) =>
  fun
  | A.Variable(name, value) =>
    [
      JavaScript_AST.Variable(
        name |> NR.get_value |> ~@Identifier.pp,
        value |> N.get_value |> gen_expression,
      ),
    ]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | A.Expression(value) => [
      is_last
        ? JavaScript_AST.Return(
            Some(value |> N.get_value |> gen_expression),
          )
        : JavaScript_AST.Expression(value |> N.get_value |> gen_expression),
    ]

and gen_unary_op = (op, value) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | A.Negative => "-"
    | A.Positive => "+"
    | A.Not => "!"
    },
    Group(value |> N.get_value |> gen_expression),
  )

and gen_binary_op = {
  let op = (symbol, lhs, rhs) =>
    JavaScript_AST.Group(
      BinaryOp(
        symbol,
        lhs |> N.get_value |> gen_expression,
        rhs |> N.get_value |> gen_expression,
      ),
    );

  fun
  | A.LogicalAnd => op("&&")
  | A.LogicalOr => op("||")
  | A.LessOrEqual => op("<=")
  | A.LessThan => op("<")
  | A.GreaterOrEqual => op(">=")
  | A.GreaterThan => op(">")
  | A.Equal => op("===")
  | A.Unequal => op("!==")
  | A.Add => op("+")
  | A.Subtract => op("-")
  | A.Multiply => op("*")
  | A.Divide => op("/")
  | A.Exponent => (
      (lhs, rhs) =>
        JavaScript_AST.FunctionCall(
          DotAccess(Identifier("Math"), "pow"),
          [
            lhs |> N.get_value |> gen_expression,
            rhs |> N.get_value |> gen_expression,
          ],
        )
    );
}

and gen_jsx =
  fun
  | A.Tag(name, attrs, values) =>
    JavaScript_AST.FunctionCall(
      __jsx_create_tag,
      [
        String(name |> NR.get_value |> ~@Identifier.pp),
        ...List.is_empty(attrs) && List.is_empty(values)
             ? []
             : [
               gen_jsx_attrs(attrs),
               ...values |> List.map(NR.get_value % gen_jsx_child),
             ],
      ],
    )

  | A.Fragment(values) =>
    JavaScript_AST.FunctionCall(
      __jsx_create_fragment,
      values |> List.map(NR.get_value % gen_jsx_child),
    )

and gen_jsx_child =
  fun
  | A.Node(value) => gen_jsx(value)
  | A.Text(value) => JavaScript_AST.String(value)
  | A.InlineExpression(value) => value |> N.get_value |> gen_expression

and gen_jsx_attrs = (attrs: list(A.jsx_attribute_t)) =>
  if (List.is_empty(attrs)) {
    JavaScript_AST.Null;
  } else {
    /* assumes that ID and unique class names / prop names only appear once at most  */
    let (classes, props) =
      attrs
      |> List.fold_left(
           ((c, p)) =>
             NR.get_value
             % (
               fun
               | A.Property(name, expr) => (
                   c,
                   [
                     (
                       name |> NR.get_value |> ~@Identifier.pp,
                       switch (expr) {
                       | Some(expr) => expr |> N.get_value |> gen_expression
                       | None =>
                         JavaScript_AST.Identifier(
                           name |> NR.get_value |> ~@Identifier.pp,
                         )
                       },
                     ),
                     ...p,
                   ],
                 )
               | A.Class(name, None) => (
                   [
                     JavaScript_AST.String(
                       name |> NR.get_value |> Fmt.str(".%a", Identifier.pp),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | A.Class(name, Some(expr)) => (
                   [
                     JavaScript_AST.Group(
                       Ternary(
                         expr |> N.get_value |> gen_expression,
                         String(
                           name
                           |> NR.get_value
                           |> Fmt.str(".%a", Identifier.pp),
                         ),
                         String(""),
                       ),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | A.ID(name) => (
                   c,
                   [
                     (
                       __id_prop,
                       String(name |> NR.get_value |> ~@Identifier.pp),
                     ),
                     ...p,
                   ],
                 )
             ),
           ([], []),
         );

    let props =
      List.is_empty(classes)
        ? props
        : [
          (
            __class_name_prop,
            {
              let rec loop =
                fun
                | [] => JavaScript_AST.String("")
                | [x] => x
                | [x, ...xs] => JavaScript_AST.BinaryOp("+", x, loop(xs));

              loop(classes);
            },
          ),
          ...props,
        ];

    JavaScript_AST.Object(props);
  };

let gen_constant = (name: A.identifier_t, value: A.expression_t) =>
  JavaScript_AST.Variable(
    name |> NR.get_value |> ~@Identifier.pp,
    value |> N.get_value |> gen_expression,
  );

let gen_function =
    (name: A.identifier_t, args: list(A.argument_t), expr: A.expression_t) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> NR.get_value |> ~@Identifier.pp),
        args
        |> List.map(
             N.get_value
             % ((A.{name}) => name |> NR.get_value |> ~@Identifier.pp),
           ),
        (
          args
          |> List.mapi((i, x) => (N.get_value(x), i))
          |> List.filter_map(
               fun
               | (A.{name, default: Some(default)}, index) =>
                 Some(
                   Assignment(
                     Identifier(name |> NR.get_value |> ~@Identifier.pp),
                     FunctionCall(
                       __knot_arg,
                       [
                         Identifier(__arguments_object),
                         Number(string_of_int(index)),
                         default |> N.get_value |> gen_expression,
                       ],
                     ),
                   ),
                 )
               | _ => None,
             )
        )
        @ (
          switch (N.get_value(expr)) {
          | Closure(stmts) =>
            let rec loop = (
              fun
              | [] => []
              | [x] => x |> N.get_value |> gen_statement(~is_last=true)
              | [x, ...xs] => (x |> N.get_value |> gen_statement) @ loop(xs)
            );

            loop(stmts);
          | raw_expr => [Return(Some(gen_expression(raw_expr)))]
          }
        ),
      ),
    )
  );

let gen_view =
    (name: A.identifier_t, props: list(A.argument_t), expr: A.expression_t) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> NR.get_value |> ~@Identifier.pp),
        [__view_props],
        (
          props
          |> List.map(N.get_value)
          |> List.mapi((index, A.{name, default}) => {
               let id = name |> NR.get_value |> ~@Identifier.pp;

               Variable(
                 id,
                 FunctionCall(
                   __knot_prop,
                   [Identifier(__view_props), String(id)]
                   @ (
                     default
                     |> Option.map(x => [x |> N.get_value |> gen_expression])
                     |?: []
                   ),
                 ),
               );
             })
        )
        @ (
          switch (N.get_value(expr)) {
          | Closure(stmts) =>
            let rec loop = (
              fun
              | [] => []
              | [x] => x |> N.get_value |> gen_statement(~is_last=true)
              | [x, ...xs] => (x |> N.get_value |> gen_statement) @ loop(xs)
            );

            loop(stmts);
          | raw_expr => [Return(Some(gen_expression(raw_expr)))]
          }
        ),
      ),
    )
  );

let gen_declaration = (name: A.identifier_t, decl: A.declaration_t) =>
  (
    switch (N.get_value(decl)) {
    | Constant(value) => [gen_constant(name, value)]
    | Function(args, expr) => [gen_function(name, args, expr)]
    | View(props, expr) => [gen_view(name, props, expr)]
    }
  )
  @ (
    switch (NR.get_value(name)) {
    | Public(name) => [JavaScript_AST.Export(name, None)]
    | _ => []
    }
  );

let generate = (resolve: resolve_t, ast: A.program_t) => {
  let resolve =
    resolve
    % (
      fun
      | "" => "."
      | relative =>
        String.starts_with("..", relative) ? relative : "./" ++ relative
    );

  let (imports, declarations) =
    ast
    |> List.fold_left(
         ((i, d)) =>
           NR.get_value
           % (
             fun
             | A.Import(namespace, imports) => (
                 i
                 @ [
                   JavaScript_AST.Import(
                     resolve(namespace),
                     imports
                     |> List.map(
                          NR.get_value
                          % (
                            fun
                            | A.MainImport(id) => (
                                __main_export,
                                Some(id |> NR.get_value |> ~@Identifier.pp),
                              )
                            | A.NamedImport(id, Some(label)) => (
                                id |> NR.get_value |> ~@Identifier.pp,
                                Some(
                                  label |> NR.get_value |> ~@Identifier.pp,
                                ),
                              )
                            | A.NamedImport(id, None) => (
                                id |> NR.get_value |> ~@Identifier.pp,
                                None,
                              )
                          ),
                        ),
                   ),
                 ],
                 d,
               )
             | A.Declaration(NamedExport(name), decl) => (
                 i,
                 d @ gen_declaration(name, decl),
               )
             | A.Declaration(MainExport(name), decl) => (
                 i,
                 d
                 @ gen_declaration(name, decl)
                 @ [
                   JavaScript_AST.Export(
                     name |> NR.get_value |> ~@Identifier.pp,
                     Some(__main_export),
                   ),
                 ],
               )
           ),
         ([], []),
       );

  List.is_empty(declarations)
    ? imports @ [JavaScript_AST.EmptyExport]
    : [
        JavaScript_AST.DefaultImport(__runtime_namespace, __util_lib),
        ...imports,
      ]
      @ declarations;
};
