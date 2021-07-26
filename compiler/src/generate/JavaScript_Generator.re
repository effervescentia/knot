/**
 Utitlities for converting module AST into JavaScript code.
 */
open Kore;
open AST;
open Reference;

let __util_lib = "$knot";
let __runtime_namespace = "@knot/runtime";
let __class_name_prop = "className";
let __id_prop = "id";
let __main_export = "main";

let _knot_util = (util, property) =>
  JavaScript_AST.DotAccess(
    DotAccess(Identifier(__util_lib), util),
    property,
  );
let _jsx_util = _knot_util("jsx");

let gen_number = x =>
  JavaScript_AST.Number(
    switch (x) {
    | Integer(value) => Int64.to_string(value)
    | Float(value, precision) => value |> Print.fmt("%.*g", precision)
    },
  );

let rec gen_expression =
  fun
  | Primitive((Boolean(x), _, _)) => JavaScript_AST.Boolean(x)
  | Primitive((Number(x), _, _)) => gen_number(x)
  | Primitive((String(x), _, _)) => JavaScript_AST.String(x)
  | Primitive((Nil, _, _)) => JavaScript_AST.Null
  | Identifier(value) =>
    JavaScript_AST.Identifier(value |> Node.value |> Identifier.to_string)
  | Group(value) =>
    JavaScript_AST.Group(value |> Node.value |> gen_expression)

  | Closure([]) => JavaScript_AST.(Null)
  | Closure(values) => {
      let rec loop = (
        fun
        | [] => []
        | [x] => gen_statement(~is_last=true, x)
        | [x, ...xs] => gen_statement(x) @ loop(xs)
      );

      values |> loop |> JavaScript_AST.iife;
    }

  | UnaryOp(op, value) => value |> gen_unary_op(op)
  | BinaryOp(op, lhs, rhs) => gen_binary_op(op, lhs, rhs)
  | JSX(value) => value |> Node.value |> gen_jsx

and gen_statement = (~is_last=false) =>
  fun
  | Variable(name, value) =>
    [
      JavaScript_AST.Variable(
        name |> Node.Raw.value |> Identifier.to_string,
        value |> Node.value |> gen_expression,
      ),
    ]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | Expression(value) => [
      is_last
        ? JavaScript_AST.Return(Some(value |> Node.value |> gen_expression))
        : JavaScript_AST.Expression(value |> Node.value |> gen_expression),
    ]

and gen_unary_op = (op, value) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | Negative => "-"
    | Positive => "+"
    | Not => "!"
    },
    Group(value |> Node.value |> gen_expression),
  )

and gen_binary_op = {
  let op = (symbol, lhs, rhs) =>
    JavaScript_AST.Group(
      BinaryOp(
        symbol,
        lhs |> Node.value |> gen_expression,
        rhs |> Node.value |> gen_expression,
      ),
    );

  fun
  | LogicalAnd => op("&&")
  | LogicalOr => op("||")
  | LessOrEqual => op("<=")
  | LessThan => op("<")
  | GreaterOrEqual => op(">=")
  | GreaterThan => op(">")
  | Equal => op("===")
  | Unequal => op("!==")
  | Add => op("+")
  | Subtract => op("-")
  | Multiply => op("*")
  | Divide => op("/")
  | Exponent => (
      (lhs, rhs) =>
        JavaScript_AST.FunctionCall(
          DotAccess(Identifier("Math"), "pow"),
          [
            lhs |> Node.value |> gen_expression,
            rhs |> Node.value |> gen_expression,
          ],
        )
    );
}

and gen_jsx =
  fun
  | Tag(name, attrs, values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createTag"),
      [
        String(name |> Node.Raw.value |> Identifier.to_string),
        ...List.is_empty(attrs) && List.is_empty(values)
             ? []
             : [
               gen_jsx_attrs(attrs),
               ...values |> List.map(Node.value % gen_jsx_child),
             ],
      ],
    )

  | Fragment(values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createFragment"),
      values |> List.map(Node.value % gen_jsx_child),
    )

and gen_jsx_child =
  fun
  | Node(value) => value |> Node.value |> gen_jsx
  | Text(value) => JavaScript_AST.String(Node.value(value))
  | InlineExpression(value) => value |> Node.value |> gen_expression

and gen_jsx_attrs = (attrs: list(jsx_attribute_t)) =>
  if (List.is_empty(attrs)) {
    JavaScript_AST.Null;
  } else {
    /* assumes that ID and unique class names / prop names only appear once at most  */
    let (classes, props) =
      attrs
      |> List.fold_left(
           ((c, p)) =>
             Node.value
             % (
               fun
               | Property(name, expr) => (
                   c,
                   [
                     (
                       name |> Node.Raw.value |> Identifier.to_string,
                       switch (expr) {
                       | Some(expr) => expr |> Node.value |> gen_expression
                       | None =>
                         JavaScript_AST.Identifier(
                           name |> Node.Raw.value |> Identifier.to_string,
                         )
                       },
                     ),
                     ...p,
                   ],
                 )
               | Class(name, None) => (
                   [
                     JavaScript_AST.String(
                       name
                       |> Node.Raw.value
                       |> Identifier.to_string
                       |> Print.fmt(".%s"),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | Class(name, Some(expr)) => (
                   [
                     JavaScript_AST.Group(
                       Ternary(
                         expr |> Node.value |> gen_expression,
                         String(
                           name
                           |> Node.Raw.value
                           |> Identifier.to_string
                           |> Print.fmt(".%s"),
                         ),
                         String(""),
                       ),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | ID(name) => (
                   c,
                   [
                     (
                       __id_prop,
                       String(name |> Node.Raw.value |> Identifier.to_string),
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

let gen_constant = (name: untyped_id_t, value: expression_t) =>
  JavaScript_AST.Variable(
    name |> Node.Raw.value |> Identifier.to_string,
    value |> Node.value |> gen_expression,
  );

let gen_function =
    (name: untyped_id_t, args: list(argument_t), expr: expression_t) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> Node.Raw.value |> Identifier.to_string),
        args
        |> List.map(
             Node.value
             % (({name}) => name |> Node.Raw.value |> Identifier.to_string),
           ),
        (
          args
          |> List.filter_map(
               Node.value
               % (
                 fun
                 | {name, default: Some(default)} =>
                   Some(
                     Assignment(
                       Identifier(
                         name |> Node.Raw.value |> Identifier.to_string,
                       ),
                       default |> Node.value |> gen_expression,
                     ),
                   )
                 | _ => None
               ),
             )
        )
        @ (
          switch (Node.value(expr)) {
          | Closure(stmts) =>
            let rec loop = (
              fun
              | [] => []
              | [x] => gen_statement(~is_last=true, x)
              | [x, ...xs] => gen_statement(x) @ loop(xs)
            );

            loop(stmts);
          | raw_expr => [Return(Some(gen_expression(raw_expr)))]
          }
        ),
      ),
    )
  );

let gen_declaration = (name: untyped_id_t, decl: declaration_t) =>
  (
    switch (Node.value(decl)) {
    | Constant(value) => [gen_constant(name, value)]
    | Function(args, expr) => [gen_function(name, args, expr)]
    }
  )
  @ (
    switch (Node.Raw.value(name)) {
    | Public(name) => [JavaScript_AST.Export(name, None)]
    | _ => []
    }
  );

let generate = (resolve: resolve_t, ast: program_t) => {
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
           fun
           | Import(namespace, imports) => (
               i
               @ [
                 JavaScript_AST.Import(
                   resolve(namespace),
                   imports
                   |> List.map(
                        fun
                        | MainImport(id) => (
                            __main_export,
                            Some(
                              id |> Node.Raw.value |> Identifier.to_string,
                            ),
                          )
                        | NamedImport(id, Some(label)) => (
                            id |> Node.Raw.value |> Identifier.to_string,
                            Some(
                              label |> Node.Raw.value |> Identifier.to_string,
                            ),
                          )
                        | NamedImport(id, None) => (
                            id |> Node.Raw.value |> Identifier.to_string,
                            None,
                          ),
                      ),
                 ),
               ],
               d,
             )
           | Declaration(NamedExport(name), decl) => (
               i,
               d @ gen_declaration(name, decl),
             )
           | Declaration(MainExport(name), decl) => (
               i,
               d
               @ gen_declaration(name, decl)
               @ [
                 JavaScript_AST.Export(
                   name |> Node.Raw.value |> Identifier.to_string,
                   Some(__main_export),
                 ),
               ],
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
