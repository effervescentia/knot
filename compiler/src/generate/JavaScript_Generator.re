/**
 Utilities for converting module AST into JavaScript code.
 */
open Kore;

module JS = JavaScript_AST;

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
  JS.DotAccess(DotAccess(Identifier(__util_lib), util), property);
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
  JS.Number(
    AST.Result.(
      switch (x) {
      | Integer(value) => Int64.to_string(value)
      | Float(value, precision) => value |> Fmt.str("%.*g", precision)
      }
    ),
  );

let rec gen_expression =
  AST.Expression.(
    fun
    | Primitive(Boolean(x)) => JS.Boolean(x)
    | Primitive(Number(x)) => gen_number(x)
    | Primitive(String(x)) => JS.String(x)
    | Primitive(Nil) => JS.Null
    | Identifier(value) =>
      value |> String.starts_with(__self)
        ? JS.DotAccess(
            Identifier(__self),
            value |> String.drop_prefix(__self),
          )
        : JS.Identifier(value)

    | Group((value, _)) => JS.Group(gen_expression(value))

    | Closure([]) => JS.Null
    | Closure(values) => {
        let rec loop = (
          fun
          | [] => []
          | [(x, _)] => x |> gen_statement(~is_last=true)
          | [(x, _), ...xs] => gen_statement(x) @ loop(xs)
        );

        values |> loop |> JS.iife;
      }

    | UnaryOp(op, value) => value |> gen_unary_op(op)
    | BinaryOp(op, lhs, rhs) => gen_binary_op(op, lhs, rhs)
    | JSX(value) => gen_jsx(value)
    | DotAccess((expr, _), (prop, _)) =>
      JS.DotAccess(gen_expression(expr), prop)
    | FunctionCall((expr, _), args) =>
      JS.FunctionCall(
        gen_expression(expr),
        args |> List.map(fst % gen_expression),
      )
    | Style(rules) => gen_style(rules)
  )

and gen_statement = (~is_last=false) =>
  AST.Expression.(
    fun
    | Variable((name, _), (value, _)) =>
      [JS.Variable(name, gen_expression(value))]
      @ (is_last ? [JS.Return(Some(Null))] : [])
    | Expression((value, _)) => [
        is_last
          ? JS.Return(Some(gen_expression(value)))
          : JS.Expression(gen_expression(value)),
      ]
  )

and gen_unary_op = (op, (value, _)) =>
  JS.UnaryOp(
    switch (op) {
    | Negative => "-"
    | Positive => "+"
    | Not => "!"
    },
    Group(gen_expression(value)),
  )

and gen_binary_op = {
  let op = (symbol, (lhs, _), (rhs, _)) =>
    JS.Group(BinaryOp(symbol, gen_expression(lhs), gen_expression(rhs)));

  AST.Operator.Binary.(
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
        ((lhs, _), (rhs, _)) =>
          JS.FunctionCall(
            DotAccess(Identifier("Math"), "pow"),
            [gen_expression(lhs), gen_expression(rhs)],
          )
      )
  );
}

and gen_jsx_element = (expr, attrs, values) =>
  JS.FunctionCall(
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
  AST.Expression.(
    fun
    | Tag((name, _), attrs, values) =>
      gen_jsx_element(String(name), attrs, values)

    | Component((id, _), attrs, values) =>
      gen_jsx_element(Identifier(id), attrs, values)

    | Fragment(values) =>
      JS.FunctionCall(
        __jsx_create_fragment,
        values |> List.map(fst % gen_jsx_child),
      )
  )

and gen_jsx_child =
  AST.Expression.(
    fun
    | Node(value) => gen_jsx(value)
    | Text(value) => JS.String(value)
    | InlineExpression((value, _)) => gen_expression(value)
  )

and gen_jsx_attrs = (attrs: list(AST.Result.jsx_attribute_t)) =>
  if (List.is_empty(attrs)) {
    JS.Null;
  } else {
    /* assumes that ID and unique class names / prop names only appear once at most  */
    let (classes, props) =
      attrs
      |> List.fold_left(
           ((c, p)) =>
             fst
             % AST.Expression.(
                 fun
                 | Property((name, _), expr) => (
                     c,
                     [
                       (
                         name,
                         switch (expr) {
                         | Some((expr, _)) => gen_expression(expr)
                         | None => JS.Identifier(name)
                         },
                       ),
                       ...p,
                     ],
                   )
                 | Class((name, _), None) => (
                     [JS.Identifier(_class_name(name)), ...c],
                     p,
                   )
                 | Class((name, _), Some((expr, _))) => (
                     [
                       JS.Group(
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
                 | ID((name, _)) => (c, [(__id_prop, String(name)), ...p])
               ),
           ([], []),
         );

    let props =
      List.is_empty(classes)
        ? props
        : [
          (__class_name_prop, JS.FunctionCall(__style_classes, classes)),
          ...props,
        ];

    JS.Object(props);
  }

and gen_style = (rules: list(AST.Result.style_rule_t)) =>
  JS.(
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

let gen_constant =
    (name: AST.Result.identifier_t, value: AST.Result.expression_t) =>
  JS.Variable(fst(name), value |> fst |> gen_expression);

let gen_enumerated =
    (
      name: AST.Result.identifier_t,
      variants:
        list(
          (
            AST.Result.identifier_t,
            list(AST.Result.node_t(AST.TypeExpression.raw_t)),
          ),
        ),
    ) => {
  let name_str = fst(name);

  JS.(
    Variable(
      name_str,
      Object(
        variants
        |> List.map(((id, args)) => {
             let variant_name = fst(id);
             let arg_ids =
               args |> List.mapi((index, _) => Util.gen_variable(index));

             (
               variant_name,
               Function(
                 Some(variant_name),
                 arg_ids,
                 [
                   Return(
                     Some(
                       Array([
                         DotAccess(Identifier(name_str), variant_name),
                         ...arg_ids |> List.map(arg_id => Identifier(arg_id)),
                       ]),
                     ),
                   ),
                 ],
               ),
             );
           }),
      ),
    )
  );
};

let gen_function =
    (
      (name, _): AST.Result.identifier_t,
      args: list(AST.Result.argument_t),
      (expr, _): AST.Result.expression_t,
    ) =>
  JS.(
    Expression(
      Function(
        Some(name),
        args
        |> List.map(fst % ((AST.Expression.{name: (name, _), _}) => name)),
        (
          args
          |> List.mapi((i, (x, _)) => (x, i))
          |> List.filter_map(
               fun
               | (
                   AST.Expression.{
                     name: (name, _),
                     default: Some(default),
                     _,
                   },
                   index,
                 ) =>
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
      (name, _): AST.Result.identifier_t,
      props: list(AST.Result.argument_t),
      mixins: list(Node.t(string, AST.Type.t)),
      (expr, _): AST.Result.expression_t,
    ) =>
  JS.(
    Expression(
      Function(
        Some(name),
        [__view_props],
        (
          props
          |> List.map(fst)
          |> List.map((AST.Expression.{name: (name, _), default, _}) => {
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
                 ...switch (Node.get_type(mixin)) {
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

let gen_declaration =
    (name: AST.Result.identifier_t, (decl, _): AST.Module.declaration_t) =>
  (
    switch (decl) {
    | Constant(value) => [gen_constant(name, value)]
    | Enumerated(variants) => [gen_enumerated(name, variants)]
    | Function(args, expr) => [gen_function(name, args, expr)]
    | View(props, mixins, expr) => [gen_view(name, props, mixins, expr)]
    }
  )
  @ [JS.Export(fst(name), None)];

let generate = (resolve: resolve_t, ast: AST.Module.program_t) => {
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
           % AST.Module.(
               fun
               | Import(namespace, imports) => (
                   i
                   @ [
                     JS.Import(
                       resolve(namespace),
                       imports
                       |> List.map(
                            fst
                            % (
                              fun
                              | MainImport((id, _)) => (
                                  __main_export,
                                  Some(id),
                                )
                              | NamedImport((id, _), Some((alias, _))) => (
                                  id,
                                  Some(alias),
                                )
                              | NamedImport((id, _), None) => (id, None)
                            ),
                          ),
                     ),
                   ],
                   d,
                 )
               | StandardImport(imports) => (
                   i
                   @ (
                     imports
                     |> List.map(
                          fst
                          % (
                            fun
                            | ((id, _), Some((alias, _))) =>
                              JS.Variable(alias, _stdlib_util(id))
                            | ((id, _), None) =>
                              JS.Variable(id, _stdlib_util(id))
                          ),
                        )
                   ),
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
                   @ [JS.Export(fst(name), Some(__main_export))],
                 )
             ),
         ([], []),
       );

  List.is_empty(declarations)
    ? imports @ [JS.EmptyExport]
    : [JS.DefaultImport(__runtime_namespace, __util_lib), ...imports]
      @ declarations;
};
