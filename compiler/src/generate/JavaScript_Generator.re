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

let number = x =>
  JavaScript_AST.Number(
    switch (x) {
    | Integer(value) => Int64.to_string(value)
    | Float(value, precision) => value |> Print.fmt("%.*g", precision)
    },
  );

let rec expression =
  fun
  | Primitive((Boolean(x), _, _)) => JavaScript_AST.Boolean(x)
  | Primitive((Number(x), _, _)) => number(x)
  | Primitive((String(x), _, _)) => JavaScript_AST.String(x)
  | Primitive((Nil, _, _)) => JavaScript_AST.Null
  | Identifier((value, _)) =>
    JavaScript_AST.Identifier(value |> Identifier.to_string)
  | Group((value, _, _)) => JavaScript_AST.Group(expression(value))

  | Closure([]) => JavaScript_AST.(Null)
  | Closure(values) => {
      let rec loop = (
        fun
        | [] => []
        | [x] => statement(~is_last=true, x)
        | [x, ...xs] => statement(x) @ loop(xs)
      );

      values |> loop |> JavaScript_AST.iife;
    }

  | UnaryOp(op, value) => value |> unary_op(op)
  | BinaryOp(op, lhs, rhs) => binary_op(op, lhs, rhs)
  | JSX((value, _)) => jsx(value)

and statement = (~is_last=false) =>
  fun
  | Variable((name, _), (value, _, _)) =>
    [
      JavaScript_AST.Variable(
        name |> Identifier.to_string,
        expression(value),
      ),
    ]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | Expression((value, _, _)) => [
      is_last
        ? JavaScript_AST.Return(Some(expression(value)))
        : JavaScript_AST.Expression(expression(value)),
    ]

and unary_op = (op, (value, _, _)) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | Negative => "-"
    | Positive => "+"
    | Not => "!"
    },
    Group(expression(value)),
  )

and binary_op = {
  let op = (symbol, (lhs, _, _), (rhs, _, _)) =>
    JavaScript_AST.Group(
      BinaryOp(symbol, expression(lhs), expression(rhs)),
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
      ((lhs, _, _), (rhs, _, _)) =>
        JavaScript_AST.FunctionCall(
          DotAccess(Identifier("Math"), "pow"),
          [expression(lhs), expression(rhs)],
        )
    );
}

and jsx =
  fun
  | Tag((name, _), attrs, values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createTag"),
      [
        String(Identifier.to_string(name)),
        ...List.is_empty(attrs) && List.is_empty(values)
             ? []
             : [jsx_attrs(attrs), ...values |> List.map(fst % jsx_child)],
      ],
    )

  | Fragment(values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createFragment"),
      values |> List.map(fst % jsx_child),
    )

and jsx_child =
  fun
  | Node((value, _)) => jsx(value)
  | Text((value, _)) => JavaScript_AST.String(value)
  | InlineExpression((value, _, _)) => expression(value)

and jsx_attrs = (attrs: list(jsx_attribute_t)) =>
  if (List.is_empty(attrs)) {
    JavaScript_AST.Null;
  } else {
    /* assumes that ID and unique class names / prop names only appear once at most  */
    let (classes, props) =
      attrs
      |> List.fold_left(
           ((c, p)) =>
             fst
             % (
               fun
               | Property((name, _), expr) => (
                   c,
                   [
                     (
                       Identifier.to_string(name),
                       switch (expr) {
                       | Some((expr, _, _)) => expression(expr)
                       | None =>
                         JavaScript_AST.Identifier(
                           Identifier.to_string(name),
                         )
                       },
                     ),
                     ...p,
                   ],
                 )
               | Class((name, _), None) => (
                   [
                     JavaScript_AST.String(
                       name |> Identifier.to_string |> Print.fmt(".%s"),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | Class((name, _), Some((expr, _, _))) => (
                   [
                     JavaScript_AST.Group(
                       Ternary(
                         expression(expr),
                         String(
                           name |> Identifier.to_string |> Print.fmt(".%s"),
                         ),
                         String(""),
                       ),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | ID((name, _)) => (
                   c,
                   [(__id_prop, String(Identifier.to_string(name))), ...p],
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

let constant = ((name, _): identifier_t, (value, _, _): expression_t) =>
  JavaScript_AST.Variable(Identifier.to_string(name), expression(value));

let function_ =
    (
      (name, _): identifier_t,
      args: list((argument_t, Type.t)),
      (expr, _, _): expression_t,
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(Identifier.to_string(name)),
        args
        |> List.map((({name}, _)) => name |> fst |> Identifier.to_string),
        (
          args
          |> List.filter_map(
               fun
               | ({name, default: Some((default, _, _))}, _) =>
                 Some(
                   Assignment(
                     Identifier(name |> fst |> Identifier.to_string),
                     expression(default),
                   ),
                 )
               | _ => None,
             )
        )
        @ (
          switch (expr) {
          | Closure(stmts) =>
            let rec loop = (
              fun
              | [] => []
              | [x] => statement(~is_last=true, x)
              | [x, ...xs] => statement(x) @ loop(xs)
            );

            loop(stmts);
          | _ => [Return(Some(expression(expr)))]
          }
        ),
      ),
    )
  );

let declaration = (name: identifier_t, decl: declaration_t) =>
  (
    switch (decl) {
    | Constant(value) => [constant(name, value)]
    | Function(args, expr) => [function_(name, args, expr)]
    }
  )
  @ (
    switch (fst(name)) {
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
                        | MainImport((id, _)) => (
                            __main_export,
                            Some(Identifier.to_string(id)),
                          )
                        | NamedImport((id, _), Some((label, _))) => (
                            Identifier.to_string(id),
                            Some(Identifier.to_string(label)),
                          )
                        | NamedImport((id, _), None) => (
                            Identifier.to_string(id),
                            None,
                          ),
                      ),
                 ),
               ],
               d,
             )
           | Declaration(NamedExport(name), decl) => (
               i,
               d @ declaration(name, decl),
             )
           | Declaration(MainExport(name), decl) => (
               i,
               d
               @ declaration(name, decl)
               @ [
                 JavaScript_AST.Export(
                   name |> fst |> Identifier.to_string,
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
