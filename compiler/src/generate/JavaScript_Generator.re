/**
 Utilities for converting module AST into JavaScript code.
 */
open Kore;
open ModuleAliases;

module A = AST.Result;
module AE = AST.Expression;
module OB = AST.Operator.Binary;

let __util_lib = "$knot";
let __runtime_namespace = "@knot/runtime";
let __class_name_prop = "className";
let __arguments_object = "arguments";
let __id_prop = "id";
let __main_export = "main";
let __view_props = "$props$";
let __self = "$";
let __style_rules = "$rules$";

let _knot_util = (util, property) =>
  JavaScript_AST.DotAccess(
    DotAccess(Identifier(__util_lib), util),
    property,
  );
let _jsx_util = _knot_util("jsx");
let _style_util = _knot_util("style");
let _platform_util = _knot_util("platform");
let _stdlib_util = _knot_util("stdlib");

let __knot_arg = _platform_util("arg");
let __knot_prop = _platform_util("prop");
let __knot_style = _platform_util("style");
let __jsx_create_tag = _jsx_util("createTag");
let __jsx_create_fragment = _jsx_util("createFragment");
let __style_classes = _style_util("classes");

let _style_name = Fmt.str("$style_%s");
let _class_name = Fmt.str("$class_%s");
let _id_name = Fmt.str("$id_%s");

let gen_number = x =>
  JavaScript_AST.Number(
    switch (x) {
    | A.Integer(value) => Int64.to_string(value)
    | A.Float(value, precision) => value |> Fmt.str("%.*g", precision)
    },
  );

let rec gen_expression =
  fun
  | AE.Primitive(Boolean(x)) => JavaScript_AST.Boolean(x)
  | AE.Primitive(Number(x)) => gen_number(x)
  | AE.Primitive(String(x)) => JavaScript_AST.String(x)
  | AE.Primitive(Nil) => JavaScript_AST.Null
  | AE.Identifier(value) =>
    value |> String.starts_with(__self)
      ? JavaScript_AST.DotAccess(
          Identifier(__self),
          value |> String.drop_prefix(__self),
        )
      : JavaScript_AST.Identifier(value)

  | AE.Group((value, _)) => JavaScript_AST.Group(gen_expression(value))

  | AE.Closure([]) => JavaScript_AST.(Null)
  | AE.Closure(values) => {
      let rec loop = (
        fun
        | [] => []
        | [(x, _)] => x |> gen_statement(~is_last=true)
        | [(x, _), ...xs] => gen_statement(x) @ loop(xs)
      );

      values |> loop |> JavaScript_AST.iife;
    }

  | AE.UnaryOp(op, value) => value |> gen_unary_op(op)
  | AE.BinaryOp(op, lhs, rhs) => gen_binary_op(op, lhs, rhs)
  | AE.JSX(value) => gen_jsx(value)
  | AE.DotAccess((expr, _), (prop, _)) =>
    JavaScript_AST.DotAccess(gen_expression(expr), prop)
  | AE.FunctionCall((expr, _), args) =>
    JavaScript_AST.FunctionCall(
      gen_expression(expr),
      args |> List.map(fst % gen_expression),
    )
  | AE.Style(rules) => gen_style(rules)

and gen_statement = (~is_last=false) =>
  fun
  | AE.Variable((name, _), (value, _)) =>
    [JavaScript_AST.Variable(name, gen_expression(value))]
    @ (is_last ? [JavaScript_AST.Return(Some(Null))] : [])
  | AE.Expression((value, _)) => [
      is_last
        ? JavaScript_AST.Return(Some(gen_expression(value)))
        : JavaScript_AST.Expression(gen_expression(value)),
    ]

and gen_unary_op = (op, (value, _)) =>
  JavaScript_AST.UnaryOp(
    switch (op) {
    | Negative => "-"
    | Positive => "+"
    | Not => "!"
    },
    Group(gen_expression(value)),
  )

and gen_binary_op = {
  let op = (symbol, (lhs, _), (rhs, _)) =>
    JavaScript_AST.Group(
      BinaryOp(symbol, gen_expression(lhs), gen_expression(rhs)),
    );

  fun
  | OB.LogicalAnd => op("&&")
  | OB.LogicalOr => op("||")
  | OB.LessOrEqual => op("<=")
  | OB.LessThan => op("<")
  | OB.GreaterOrEqual => op(">=")
  | OB.GreaterThan => op(">")
  | OB.Equal => op("===")
  | OB.Unequal => op("!==")
  | OB.Add => op("+")
  | OB.Subtract => op("-")
  | OB.Multiply => op("*")
  | OB.Divide => op("/")
  | OB.Exponent => (
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
  | AE.Tag((name, _), attrs, values) =>
    gen_jsx_element(String(name), attrs, values)

  | AE.Component((id, _), attrs, values) =>
    gen_jsx_element(Identifier(id), attrs, values)

  | AE.Fragment(values) =>
    JavaScript_AST.FunctionCall(
      __jsx_create_fragment,
      values |> List.map(fst % gen_jsx_child),
    )

and gen_jsx_child =
  fun
  | AE.Node(value) => gen_jsx(value)
  | AE.Text(value) => JavaScript_AST.String(value)
  | AE.InlineExpression((value, _)) => gen_expression(value)

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
               | AE.Property((name, _), expr) => (
                   c,
                   [
                     (
                       name,
                       switch (expr) {
                       | Some((expr, _)) => gen_expression(expr)
                       | None => JavaScript_AST.Identifier(name)
                       },
                     ),
                     ...p,
                   ],
                 )
               | AE.Class((name, _), None) => (
                   [JavaScript_AST.Identifier(_class_name(name)), ...c],
                   p,
                 )
               | AE.Class((name, _), Some((expr, _))) => (
                   [
                     JavaScript_AST.Group(
                       Ternary(
                         gen_expression(expr),
                         Identifier(_class_name(name)),
                         String(""),
                       ),
                     ),
                     ...c,
                   ],
                   p,
                 )
               | AE.ID((name, _)) => (
                   c,
                   [(__id_prop, String(name)), ...p],
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
            JavaScript_AST.FunctionCall(__style_classes, classes),
          ),
          ...props,
        ];

    JavaScript_AST.Object(props);
  }

and gen_style = (rules: list(A.style_rule_t)) =>
  JavaScript_AST.(
    iife([
      Variable(__self, _style_util("styleExpressionPlugin")),
      Variable(__style_rules, _style_util("styleRulePlugin")),
      Return(
        Object(
          rules
          |> List.map(
               fst
               % (
                 (((key, _), (value, _))) => (
                   key,
                   FunctionCall(
                     DotAccess(Identifier(__style_rules), key),
                     [gen_expression(value)],
                   ),
                 )
               ),
             ),
        )
        |> Option.some,
      ),
    ])
  );

let gen_constant = (name: A.identifier_t, value: A.expression_t) =>
  JavaScript_AST.Variable(fst(name), value |> fst |> gen_expression);

let gen_enumerated =
    (
      name: A.identifier_t,
      variants:
        list((A.identifier_t, list(A.node_t(AST.TypeExpression.raw_t)))),
    ) => {
  let name_str = fst(name);

  JavaScript_AST.Variable(
    name_str,
    Object(
      variants
      |> List.map(((id, args)) => {
           let variant_name = fst(id);
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
        Some(name),
        args |> List.map(fst % ((AE.{name: (name, _), _}) => name)),
        (
          args
          |> List.mapi((i, (x, _)) => (x, i))
          |> List.filter_map(
               fun
               | (AE.{name: (name, _), default: Some(default), _}, index) =>
                 Some(
                   Assignment(
                     Identifier(name),
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
      mixins: list(N.t(string, AST.Type.t)),
      (expr, _): A.expression_t,
    ) =>
  JavaScript_AST.(
    Expression(
      Function(
        Some(name),
        [__view_props],
        (
          props
          |> List.map(fst)
          |> List.map((AE.{name: (name, _), default, _}) => {
               Variable(
                 name,
                 FunctionCall(
                   __knot_prop,
                   [Identifier(__view_props), String(name)]
                   @ (default |?> (((x, _)) => [gen_expression(x)]) |?: []),
                 ),
               )
             })
        )
        @ (
          mixins
          |> List.concat_map(mixin => {
               let mixin_name = fst(mixin);
               let style_name = _style_name(mixin_name);

               [
                 Variable(
                   style_name,
                   FunctionCall(
                     __knot_style,
                     [
                       DotAccess(Identifier(__util_lib), "style"),
                       FunctionCall(Identifier(mixin_name), []),
                     ],
                   ),
                 ),
                 ...switch (N.get_type(mixin)) {
                    // TODO: add state mixin support here

                    | _ => []
                    },
               ];
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

let gen_declaration = (name: A.identifier_t, (decl, _): A.declaration_t) =>
  (
    switch (decl) {
    | Constant(value) => [gen_constant(name, value)]
    | Enumerated(variants) => [gen_enumerated(name, variants)]
    | Function(args, expr) => [gen_function(name, args, expr)]
    | View(props, mixins, expr) => [gen_view(name, props, mixins, expr)]
    }
  )
  @ [JavaScript_AST.Export(fst(name), None)];

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
                                Some(id),
                              )
                            | A.NamedImport((id, _), Some((alias, _))) => (
                                id,
                                Some(alias),
                              )
                            | A.NamedImport((id, _), None) => (id, None)
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
                            JavaScript_AST.Variable(alias, _stdlib_util(id))
                          | ((id, _), None) =>
                            JavaScript_AST.Variable(id, _stdlib_util(id))
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
                 @ [JavaScript_AST.Export(fst(name), Some(__main_export))],
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
