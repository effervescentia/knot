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
let _stdlib_util = _knot_util("stdlib");

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
  | A.Group((value, _)) => JavaScript_AST.Group(gen_expression(value))

  | A.Closure([]) => JavaScript_AST.(Null)
  | A.Closure(values) => {
      let rec loop = (
        fun
        | [] => []
        | [(x, _)] => x |> gen_statement(~is_last=true)
        | [(x, _), ...xs] => gen_statement(x) @ loop(xs)
      );

      values |> loop |> JavaScript_AST.iife;
    }

  | A.UnaryOp(op, value) => value |> gen_unary_op(op)
  | A.BinaryOp(op, lhs, rhs) => gen_binary_op(op, lhs, rhs)
  | A.JSX(value) => gen_jsx(value)
  | A.DotAccess((expr, _), (prop, _)) =>
    JavaScript_AST.DotAccess(gen_expression(expr), prop)
  | A.FunctionCall((expr, _), args) =>
    JavaScript_AST.FunctionCall(
      gen_expression(expr),
      args |> List.map(fst % gen_expression),
    )

and gen_statement = (~is_last=false) =>
  fun
  | A.Variable((name, _), (value, _)) =>
    [
      JavaScript_AST.Variable(name |> ~@Identifier.pp, gen_expression(value)),
    ]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | A.Expression((value, _)) => [
      is_last
        ? JavaScript_AST.Return(Some(gen_expression(value)))
        : JavaScript_AST.Expression(gen_expression(value)),
    ]

and gen_unary_op = (op, (value, _)) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | A.Negative => "-"
    | A.Positive => "+"
    | A.Not => "!"
    },
    Group(gen_expression(value)),
  )

and gen_binary_op = {
  let op = (symbol, (lhs, _), (rhs, _)) =>
    JavaScript_AST.Group(
      BinaryOp(symbol, gen_expression(lhs), gen_expression(rhs)),
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
      ((lhs, _), (rhs, _)) =>
        JavaScript_AST.FunctionCall(
          DotAccess(Identifier("Math"), "pow"),
          [gen_expression(lhs), gen_expression(rhs)],
        )
    );
}

and gen_jsx_element = (expr, attrs, values) =>
  JavaScript_AST.FunctionCall(
    __jsx_create_tag,
    [
      expr,
      ...List.is_empty(attrs) && List.is_empty(values)
           ? []
           : [
             gen_jsx_attrs(attrs),
             ...values |> List.map(fst % gen_jsx_child),
           ],
    ],
  )

and gen_jsx =
  fun
  | A.Tag((name, _), attrs, values) =>
    gen_jsx_element(String(name |> ~@Identifier.pp), attrs, values)

  | A.Component((id, _), attrs, values) =>
    gen_jsx_element(Identifier(id |> ~@Identifier.pp), attrs, values)

  | A.Fragment(values) =>
    JavaScript_AST.FunctionCall(
      __jsx_create_fragment,
      values |> List.map(fst % gen_jsx_child),
    )

and gen_jsx_child =
  fun
  | A.Node(value) => gen_jsx(value)
  | A.Text(value) => JavaScript_AST.String(value)
  | A.InlineExpression((value, _)) => gen_expression(value)

and gen_jsx_attrs = (attrs: list(A.jsx_attribute_t)) =>
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
               | A.Property((name, _), expr) => (
                   c,
                   [
                     (
                       name |> ~@Identifier.pp,
                       switch (expr) {
                       | Some((expr, _)) => gen_expression(expr)
                       | None =>
                         JavaScript_AST.Identifier(name |> ~@Identifier.pp)
                       },
                     ),
                     ...p,
                   ],
                 )
               | A.Class((name, _), None) => (
                   [
                     JavaScript_AST.String(
                       Fmt.str(".%a", Identifier.pp, name),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | A.Class((name, _), Some((expr, _))) => (
                   [
                     JavaScript_AST.Group(
                       Ternary(
                         gen_expression(expr),
                         String(Fmt.str(".%a", Identifier.pp, name)),
                         String(""),
                       ),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | A.ID((name, _)) => (
                   c,
                   [(__id_prop, String(name |> ~@Identifier.pp)), ...p],
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
    name |> fst |> ~@Identifier.pp,
    value |> fst |> gen_expression,
  );

let gen_enumerated =
    (
      name: A.identifier_t,
      variants:
        list((A.identifier_t, list(A.node_t(A.TypeExpression.raw_t)))),
    ) => {
  let name_str = name |> fst |> ~@Identifier.pp;

  JavaScript_AST.Variable(
    name_str,
    Object(
      variants
      |> List.map(((id, args)) => {
           let variant_name = id |> fst |> ~@Identifier.pp;
           let arg_ids =
             args |> List.mapi((index, _) => Util.gen_variable(index));

           (
             variant_name,
             JavaScript_AST.Function(
               Some(variant_name),
               arg_ids,
               [
                 Return(
                   Some(
                     Array([
                       DotAccess(Identifier(name_str), variant_name),
                       ...arg_ids
                          |> List.map(arg_id =>
                               JavaScript_AST.Identifier(arg_id)
                             ),
                     ]),
                   ),
                 ),
               ],
             ),
           );
         }),
    ),
  );
};

let gen_function =
    (
      (name, _): A.identifier_t,
      args: list(A.argument_t),
      (expr, _): A.expression_t,
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> ~@Identifier.pp),
        args
        |> List.map(
             fst % ((A.{name: (name, _)}) => name |> ~@Identifier.pp),
           ),
        (
          args
          |> List.mapi((i, (x, _)) => (x, i))
          |> List.filter_map(
               fun
               | (A.{name: (name, _), default: Some(default)}, index) =>
                 Some(
                   Assignment(
                     Identifier(name |> ~@Identifier.pp),
                     FunctionCall(
                       __knot_arg,
                       [
                         Identifier(__arguments_object),
                         Number(string_of_int(index)),
                         default |> fst |> gen_expression,
                       ],
                     ),
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
              | [(x, _)] => x |> gen_statement(~is_last=true)
              | [(x, _), ...xs] => gen_statement(x) @ loop(xs)
            );

            loop(stmts);
          | raw_expr => [Return(Some(gen_expression(raw_expr)))]
          }
        ),
      ),
    )
  );

let gen_view =
    (
      (name, _): A.identifier_t,
      props: list(A.argument_t),
      (expr, _): A.expression_t,
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> ~@Identifier.pp),
        [__view_props],
        (
          props
          |> List.map(fst)
          |> List.mapi((index, A.{name: (name, _), default}) => {
               let id = name |> ~@Identifier.pp;

               Variable(
                 id,
                 FunctionCall(
                   __knot_prop,
                   [Identifier(__view_props), String(id)]
                   @ (default |?> (((x, _)) => [gen_expression(x)]) |?: []),
                 ),
               );
             })
        )
        @ (
          switch (expr) {
          | Closure(stmts) =>
            let rec loop = (
              fun
              | [] => []
              | [(x, _)] => x |> gen_statement(~is_last=true)
              | [(x, _), ...xs] => gen_statement(x) @ loop(xs)
            );

            loop(stmts);
          | raw_expr => [Return(Some(gen_expression(raw_expr)))]
          }
        ),
      ),
    )
  );

let gen_style =
    (
      (name, _): A.identifier_t,
      args: list(A.argument_t),
      rule_sets: list(A.style_rule_set_t),
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name |> ~@Identifier.pp),
        [__view_props],
        (
          args
          |> List.map(fst)
          |> List.mapi((index, A.{name: (name, _), default}) => {
               let id = name |> ~@Identifier.pp;

               Variable(
                 id,
                 FunctionCall(
                   __knot_prop,
                   [Identifier(__view_props), String(id)]
                   @ (default |?> (((x, _)) => [gen_expression(x)]) |?: []),
                 ),
               );
             })
        )
        @ [
          Return(
            Object(
              rule_sets
              |> List.map(
                   fst
                   % (
                     ((matcher, rules)) => (
                       switch (matcher) {
                       | A.Class((id, _)) =>
                         Fmt.str(".%a", Identifier.pp, id)
                       | A.ID((id, _)) => Fmt.str("#%a", Identifier.pp, id)
                       },
                       Object(
                         rules
                         |> List.map(
                              fst
                              % (
                                (((key, _), (value, _))) => (
                                  key |> ~@Identifier.pp,
                                  value |> gen_expression,
                                )
                              ),
                            ),
                       ),
                     )
                   ),
                 ),
            )
            |> Option.some,
          ),
        ],
      ),
    )
  );

let gen_declaration = (name: A.identifier_t, (decl, _): A.declaration_t) =>
  (
    switch (decl) {
    | Constant(value) => [gen_constant(name, value)]
    | Enumerated(variants) => [gen_enumerated(name, variants)]
    | Function(args, expr) => [gen_function(name, args, expr)]
    | View(props, expr) => [gen_view(name, props, expr)]
    | Style(args, rule_sets) => [gen_style(name, args, rule_sets)]
    }
  )
  @ (
    switch (fst(name)) {
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
           fst
           % (
             fun
             | A.Import(namespace, imports) => (
                 i
                 @ [
                   JavaScript_AST.Import(
                     resolve(namespace),
                     imports
                     |> List.map(
                          fst
                          % (
                            fun
                            | A.MainImport((id, _)) => (
                                __main_export,
                                Some(id |> ~@Identifier.pp),
                              )
                            | A.NamedImport((id, _), Some((alias, _))) => (
                                id |> ~@Identifier.pp,
                                Some(alias |> ~@Identifier.pp),
                              )
                            | A.NamedImport((id, _), None) => (
                                id |> ~@Identifier.pp,
                                None,
                              )
                          ),
                        ),
                   ),
                 ],
                 d,
               )
             | A.StandardImport(imports) => (
                 i
                 @ (
                   imports
                   |> List.map(
                        fst
                        % (
                          fun
                          | ((id, _), Some((alias, _))) =>
                            JavaScript_AST.Variable(
                              alias |> ~@Identifier.pp,
                              id |> ~@Identifier.pp |> _stdlib_util,
                            )
                          | ((id, _), None) => {
                              let name = id |> ~@Identifier.pp;

                              JavaScript_AST.Variable(
                                name,
                                _stdlib_util(name),
                              );
                            }
                        ),
                      )
                 ),
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
                     name |> fst |> ~@Identifier.pp,
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
