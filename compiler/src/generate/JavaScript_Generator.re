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
    | Integer(value) => value |> Int64.to_string
    | Float(value, precision) => value |> Print.fmt("%.*g", precision)
    },
  );

let rec expression =
  fun
  | Primitive((Boolean(x), _, _)) => JavaScript_AST.Boolean(x)
  | Primitive((Number(x), _, _)) => x |> number
  | Primitive((String(x), _, _)) => JavaScript_AST.String(x)
  | Primitive((Nil, _, _)) => JavaScript_AST.Null
  | Identifier((value, _)) =>
    JavaScript_AST.Identifier(value |> Identifier.to_string)
  | Group((value, _, _)) => JavaScript_AST.Group(value |> expression)

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
  | JSX((value, _)) => value |> jsx

and statement = (~is_last=false) =>
  fun
  | Variable((name, _), (value, _, _)) =>
    [
      JavaScript_AST.Variable(
        name |> Identifier.to_string,
        value |> expression,
      ),
    ]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | Expression((value, _, _)) => [
      is_last
        ? JavaScript_AST.Return(Some(value |> expression))
        : JavaScript_AST.Expression(value |> expression),
    ]

and unary_op = (op, (value, _, _)) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | Negative => "-"
    | Positive => "+"
    | Not => "!"
    },
    Group(value |> expression),
  )

and binary_op = {
  let op = (symbol, (lhs, _, _), (rhs, _, _)) =>
    JavaScript_AST.Group(
      BinaryOp(symbol, lhs |> expression, rhs |> expression),
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
          [lhs |> expression, rhs |> expression],
        )
    );
}

and jsx =
  fun
  | Tag((name, _), attrs, values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createTag"),
      [
        String(name |> Identifier.to_string),
        ...List.is_empty(attrs) && List.is_empty(values)
             ? []
             : [attrs |> jsx_attrs, ...values |> List.map(fst % jsx_child)],
      ],
    )

  | Fragment(values) =>
    JavaScript_AST.FunctionCall(
      _jsx_util("createFragment"),
      values |> List.map(fst % jsx_child),
    )

and jsx_child =
  fun
  | Node((value, _)) => value |> jsx
  | Text((value, _)) => JavaScript_AST.String(value)
  | InlineExpression((value, _, _)) => value |> expression

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
                       name |> Identifier.to_string,
                       switch (expr) {
                       | Some((expr, _, _)) => expr |> expression
                       | None =>
                         JavaScript_AST.Identifier(
                           name |> Identifier.to_string,
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
                         expr |> expression,
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
                   [
                     (__id_prop, String(name |> Identifier.to_string)),
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

let constant = ((name, _): identifier_t, (value, _, _): expression_t) =>
  JavaScript_AST.Variable(name |> Identifier.to_string, value |> expression);

let function_ =
    (
      (name, _): identifier_t,
      args: list((argument_t, Type.t)),
      (expr, _, _): expression_t,
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> Identifier.to_string),
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
                     default |> expression,
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

            stmts |> loop;
          | _ => [Return(Some(expr |> expression))]
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
    switch (name |> fst) {
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
                   namespace |> resolve,
                   imports
                   |> List.map(
                        fun
                        | MainImport((id, _)) => (
                            __main_export,
                            Some(id |> Identifier.to_string),
                          )
                        | NamedImport((id, _), Some((label, _))) => (
                            id |> Identifier.to_string,
                            Some(label |> Identifier.to_string),
                          )
                        | NamedImport((id, _), None) => (
                            id |> Identifier.to_string,
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

  declarations |> List.is_empty
    ? imports @ [JavaScript_AST.EmptyExport]
    : [
        JavaScript_AST.DefaultImport(__runtime_namespace, __util_lib),
        ...imports,
      ]
      @ declarations;
};
