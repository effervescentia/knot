/**
 Types and utilities for a module's Abstract Syntax Tree.
 */
open Infix;
open Reference;

include AST_Common;

module TypeExpression = AST_TypeExpression;
module TypeDefinition = AST_TypeDefinition;

/**
 abstraction on the type of the nodes that makeup an AST
 */
module type ASTParams = {
  type type_t;

  let node_to_entity:
    (
      ~attributes: list(Pretty.XML.xml_attr_t(string))=?,
      ~children: list(Dump.Entity.t)=?,
      string,
      N.t('a, type_t)
    ) =>
    Dump.Entity.t;
};

/**
 constructor for AST modules
 */
module Make = (Params: ASTParams) => {
  include AST_Common;

  let typed_node_to_entity = Params.node_to_entity;

  /**
   type for AST expression nodes
   */
  type type_t = Params.type_t;

  /**
   container for AST nodes
   */
  type node_t('a) = N.t('a, type_t);

  /**
   a JSX AST node
   */
  type jsx_t =
    | Tag(identifier_t, list(jsx_attribute_t), list(jsx_child_t))
    | Component(node_t(string), list(jsx_attribute_t), list(jsx_child_t))
    | Fragment(list(jsx_child_t))

  /**
   a JSX child AST node
   */
  and jsx_child_t = untyped_t(raw_jsx_child_t)
  /**
   supported JSX children
   */
  and raw_jsx_child_t =
    | Text(string)
    | Node(jsx_t)
    | InlineExpression(expression_t)

  /**
   a JSX attribute AST node
   */
  and jsx_attribute_t = untyped_t(raw_jsx_attribute_t)
  /**
   supported JSX attributes
   */
  and raw_jsx_attribute_t =
    | ID(identifier_t)
    | Class(identifier_t, option(expression_t))
    | Property(identifier_t, option(expression_t))

  /**
   an expression AST node
   */
  and expression_t = node_t(raw_expression_t)
  /**
   supported expressions and type containers
   */
  and raw_expression_t =
    | Primitive(primitive_t)
    | Identifier(string)
    | JSX(jsx_t)
    | Group(expression_t)
    | BinaryOp(binary_t, expression_t, expression_t)
    | UnaryOp(unary_t, expression_t)
    | Closure(list(statement_t))
    | DotAccess(expression_t, untyped_t(string))
    | FunctionCall(expression_t, list(expression_t))

  /**
   a statement AST node
   */
  and statement_t = node_t(raw_statement_t)
  /**
   supported statement types
   */
  and raw_statement_t =
    | Variable(identifier_t, expression_t)
    | Expression(expression_t);

  /**
   an AST node of an argument for a functional closure
   */
  type argument_t = node_t(raw_argument_t)
  /**
   a node of an argument for a functional closure
   */
  and raw_argument_t = {
    name: identifier_t,
    default: option(expression_t),
    type_: option(TypeExpression.t),
  };

  /* tag helpers */

  let of_internal = namespace => Namespace.Internal(namespace);
  let of_external = namespace => Namespace.External(namespace);

  let of_var = ((name, x)) => Variable(name, x);
  let of_expr = x => Expression(x);
  let of_id = x => Identifier(x);
  let of_group = x => Group(x);
  let of_closure = xs => Closure(xs);
  let of_dot_access = ((expr, prop)) => DotAccess(expr, prop);
  let of_func_call = ((expr, args)) => FunctionCall(expr, args);

  let of_unary_op = ((op, x)) => UnaryOp(op, x);
  let of_not_op = x => (Not, x) |> of_unary_op;
  let of_neg_op = x => (Negative, x) |> of_unary_op;
  let of_pos_op = x => (Positive, x) |> of_unary_op;

  let of_binary_op = ((op, l, r)) => BinaryOp(op, l, r);
  let of_and_op = ((l, r)) => (LogicalAnd, l, r) |> of_binary_op;
  let of_or_op = ((l, r)) => (LogicalOr, l, r) |> of_binary_op;

  let of_mult_op = ((l, r)) => (Multiply, l, r) |> of_binary_op;
  let of_div_op = ((l, r)) => (Divide, l, r) |> of_binary_op;
  let of_add_op = ((l, r)) => (Add, l, r) |> of_binary_op;
  let of_sub_op = ((l, r)) => (Subtract, l, r) |> of_binary_op;

  let of_lt_op = ((l, r)) => (LessThan, l, r) |> of_binary_op;
  let of_lte_op = ((l, r)) => (LessOrEqual, l, r) |> of_binary_op;
  let of_gt_op = ((l, r)) => (GreaterThan, l, r) |> of_binary_op;
  let of_gte_op = ((l, r)) => (GreaterOrEqual, l, r) |> of_binary_op;

  let of_eq_op = ((l, r)) => (Equal, l, r) |> of_binary_op;
  let of_ineq_op = ((l, r)) => (Unequal, l, r) |> of_binary_op;

  let of_expo_op = ((l, r)) => (Exponent, l, r) |> of_binary_op;

  let of_jsx = x => JSX(x);
  let of_frag = xs => Fragment(xs);
  let of_tag = ((name, attrs, children)) => Tag(name, attrs, children);
  let of_component = ((id, attrs, children)) =>
    Component(id, attrs, children);
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

  module Dump = {
    open Pretty.Formatters;

    include AST_Common.Dump;

    let rec expr_to_entity = expr =>
      (
        switch (fst(expr)) {
        | Primitive(prim) =>
          typed_node_to_entity(
            ~attributes=[("value", prim_to_string(prim))],
            "Primitive",
          )

        | Identifier(id) =>
          typed_node_to_entity(~attributes=[("value", id)], "Identifier")

        | JSX(jsx) =>
          typed_node_to_entity(~children=[jsx_to_entity(jsx)], "JSX")

        | Group(group) =>
          typed_node_to_entity(~children=[expr_to_entity(group)], "Group")

        | Closure(stmts) =>
          typed_node_to_entity(
            ~children=
              stmts
              |> List.map(stmt =>
                   typed_node_to_entity(
                     ~children=[stmt_to_entity(stmt)],
                     "Statement",
                     stmt,
                   )
                 ),
            "Closure",
          )

        | BinaryOp(op, lhs, rhs) =>
          typed_node_to_entity(
            ~children=[
              typed_node_to_entity(
                ~children=[expr_to_entity(lhs)],
                "LHS",
                lhs,
              ),
              typed_node_to_entity(
                ~children=[expr_to_entity(rhs)],
                "RHS",
                rhs,
              ),
            ],
            binary_to_string(op),
          )

        | UnaryOp(op, expr) =>
          typed_node_to_entity(
            ~children=[expr_to_entity(expr)],
            unary_to_string(op),
          )

        | DotAccess(expr, (prop, _)) =>
          typed_node_to_entity(
            ~attributes=[("name", prop)],
            ~children=[expr_to_entity(expr)],
            "DotAccess",
          )

        | FunctionCall(expr, args) =>
          typed_node_to_entity(
            ~children=[
              Entity.create(~children=[expr_to_entity(expr)], "Function"),
              Entity.create(
                ~children=args |> List.map(expr_to_entity),
                "Arguments",
              ),
            ],
            "FunctionCall",
          )
        }
      )(
        expr,
      )

    and jsx_to_entity =
      fun
      | Tag(name, attrs, children) =>
        Entity.create(
          ~children=[
            id_to_entity("Name", name),
            Entity.create(
              ~children=attrs |> List.map(jsx_attr_to_entity),
              "Attributes",
            ),
            Entity.create(
              ~children=children |> List.map(jsx_child_to_entity),
              "Children",
            ),
          ],
          "Tag",
        )

      | Component(view, attrs, children) =>
        Entity.create(
          ~children=[
            view
            |> typed_node_to_entity(
                 ~attributes=[("name", fst(view))],
                 "Identifier",
               ),
            Entity.create(
              ~children=attrs |> List.map(jsx_attr_to_entity),
              "Attributes",
            ),
            Entity.create(
              ~children=children |> List.map(jsx_child_to_entity),
              "Children",
            ),
          ],
          "Component",
        )

      | Fragment(children) =>
        Entity.create(
          ~children=children |> List.map(jsx_child_to_entity),
          "Fragment",
        )

    and jsx_child_to_entity = jsx_child =>
      (
        switch (fst(jsx_child)) {
        | Text(text) =>
          untyped_node_to_entity(~attributes=[("value", text)], "Text")

        | Node(jsx) =>
          untyped_node_to_entity(~children=[jsx_to_entity(jsx)], "Node")

        | InlineExpression(expr) =>
          untyped_node_to_entity(
            ~children=[expr_to_entity(expr)],
            "InlineExpr",
          )
        }
      )(
        jsx_child,
      )

    and jsx_attr_to_entity = attr =>
      (
        (
          switch (fst(attr)) {
          | Class(name, value) => ("Class", name, value)

          | ID(name) => ("ID", name, None)

          | Property(name, value) => ("Property", name, value)
          }
        )
        |> Tuple.map_snd3(id_to_entity("Name"))
        |> (
          fun
          | (entity, name_child, Some(expr)) =>
            untyped_node_to_entity(
              ~children=[name_child, expr_to_entity(expr)],
              entity,
            )
          | (entity, name_child, None) =>
            untyped_node_to_entity(~children=[name_child], entity)
        )
      )(
        attr,
      )

    and stmt_to_entity = stmt =>
      (
        switch (fst(stmt)) {
        | Variable(name, expr) =>
          typed_node_to_entity(
            "Variable",
            ~children=[id_to_entity("Name", name), expr_to_entity(expr)],
          )

        | Expression(expr) =>
          typed_node_to_entity(
            "Expression",
            ~children=[expr_to_entity(expr)],
          )
        }
      )(
        stmt,
      );
  };
};

module Raw =
  Make({
    type type_t = Type.Raw.t;

    let node_to_entity = (~attributes=[], ~children=[], label, node) =>
      Dump.node_to_entity(Type.Raw.pp, ~attributes, ~children, label, node);
  });

include Make({
  type type_t = Type.t;

  let node_to_entity = (~attributes=[], ~children=[], label, node) =>
    Dump.node_to_entity(Type.pp, ~attributes, ~children, label, node);
});

type style_matcher_t =
  | MatchClass(identifier_t)
  | MatchID(identifier_t);

type style_rule_t = untyped_t(raw_style_rule_t)
and raw_style_rule_t = (identifier_t, expression_t);

type style_rule_set_t = untyped_t(raw_style_rule_set_t)
and raw_style_rule_set_t = (style_matcher_t, list(style_rule_t));

/**
 a declaration AST node
 */
type declaration_t = node_t(raw_declaration_t)
/**
 supported module declarations
 */
and raw_declaration_t =
  | Constant(expression_t)
  | Enumerated(list((identifier_t, list(node_t(TypeExpression.raw_t)))))
  | Function(list(argument_t), expression_t)
  | View(list(argument_t), expression_t)
  | Style(list(argument_t), list(style_rule_set_t));

/**
 a stdlib import AST node
 */
type standard_import_t = untyped_t((identifier_t, option(identifier_t)));

/**
 an import AST node
 */
type import_t = untyped_t(raw_import_t)
/**
 supported import types
 */
and raw_import_t =
  | MainImport(identifier_t)
  | NamedImport(identifier_t, option(identifier_t));

/**
 supported export types
 */
type export_t =
  | MainExport(identifier_t)
  | NamedExport(identifier_t);

/**
 module statement AST node
 */
type module_statement_t = untyped_t(raw_module_statement_t)
/**
 supported top-level module statements
 */
and raw_module_statement_t =
  | StandardImport(list(standard_import_t))
  | Import(Namespace.t, list(import_t))
  | Declaration(export_t, declaration_t);

/**
 the AST of an entire module
 */
type program_t = list(module_statement_t);

/* tag helpers */

let of_main_import = x => MainImport(x);
let of_named_import = ((x, y)) => NamedImport(x, y);

let of_main_export = x => MainExport(x);
let of_named_export = x => NamedExport(x);

let of_class_matcher = x => MatchClass(x);
let of_id_matcher = x => MatchID(x);

let of_const = x => Constant(x);
let of_enum = variants => Enumerated(variants);
let of_func = ((args, expr)) => Function(args, expr);
let of_view = ((props, expr)) => View(props, expr);
let of_style = ((args, rule_sets)) => Style(args, rule_sets);

let of_standard_import = imports => StandardImport(imports);
let of_import = ((namespace, imports)) => Import(namespace, imports);
let of_decl = ((name, x)) => Declaration(name, x);

module Dump = {
  include Dump;

  open Pretty.Formatters;

  let export_to_entity =
    fun
    | MainExport(id) => id_to_entity("MainExport", id)
    | NamedExport(id) => id_to_entity("NamedExport", id);

  let style_matcher_to_entity =
    fun
    | MatchClass(id) => id_to_entity("ClassMatcher", id)
    | MatchID(id) => id_to_entity("IDMatcher", id);

  let argument_to_entity = (label, arg) => {
    let {name, default, type_} = fst(arg);
    let children = ref([id_to_entity("Name", name)]);

    default
    |> Option.iter(default =>
         children := [expr_to_entity(default), ...children^]
       );

    type_
    |> Option.iter(type_ =>
         children := [untyped_node_to_entity("Type", type_), ...children^]
       );

    typed_node_to_entity(~children=children^, label, arg);
  };

  let decl_to_entity: declaration_t => Entity.t =
    decl =>
      switch (fst(decl)) {
      | Constant(expr) =>
        typed_node_to_entity(
          ~children=[expr_to_entity(expr)],
          "Constant",
          decl,
        )

      | Enumerated(variants) =>
        typed_node_to_entity(
          ~children=
            variants
            |> List.map(((name, args)) =>
                 Entity.create(
                   ~attributes=[("name", fst(name))],
                   ~children=
                     args
                     |> List.map(arg =>
                          typed_node_to_entity(
                            ~children=[
                              N.drop_type(arg)
                              |> TypeExpression.Dump.to_entity,
                            ],
                            "Argument",
                            arg,
                          )
                        ),
                   "Variant",
                 )
               ),
          "Enumerated",
          decl,
        )

      | Function(args, expr) =>
        typed_node_to_entity(
          ~children=[
            Entity.create(
              ~children=args |> List.map(argument_to_entity("Argument")),
              "Arguments",
            ),
            Entity.create(~children=[expr_to_entity(expr)], "Body"),
          ],
          "Function",
          decl,
        )

      | View(props, expr) =>
        typed_node_to_entity(
          ~children=[
            Entity.create(
              ~children=props |> List.map(argument_to_entity("Property")),
              "Properties",
            ),
            Entity.create(~children=[expr_to_entity(expr)], "Body"),
          ],
          "View",
          decl,
        )

      | Style(props, rule_sets) =>
        typed_node_to_entity(
          ~children=[
            Entity.create(
              ~children=props |> List.map(argument_to_entity("Property")),
              "Properties",
            ),
            Entity.create(
              ~children=
                rule_sets
                |> List.map((((matcher, rules), _) as rule_set) =>
                     untyped_node_to_entity(
                       ~children=[
                         style_matcher_to_entity(matcher),
                         Entity.create(
                           ~children=
                             rules
                             |> List.map((((key, value), _) as rule) =>
                                  untyped_node_to_entity(
                                    ~children=[
                                      id_to_entity("Key", key),
                                      Entity.create(
                                        ~children=[expr_to_entity(value)],
                                        "Value",
                                      ),
                                    ],
                                    "Rule",
                                    rule,
                                  )
                                ),
                           "Rules",
                         ),
                       ],
                       "RuleSet",
                       rule_set,
                     )
                   ),
              "RuleSets",
            ),
          ],
          "Style",
          decl,
        )
      };

  let import_to_entity = import =>
    switch (fst(import)) {
    | MainImport(name) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "MainImport",
        import,
      )

    | NamedImport(name, Some(alias)) =>
      untyped_node_to_entity(
        ~children=[
          id_to_entity("Name", name),
          Entity.create(
            ~range=N.get_range(alias),
            ~attributes=[("value", fst(alias))],
            "Alias",
          ),
        ],
        "NamedImport",
        import,
      )

    | NamedImport(name, None) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "NamedImport",
        import,
      )
    };

  let standard_import_to_entity = import =>
    switch (fst(import)) {
    | (name, Some(alias)) =>
      untyped_node_to_entity(
        ~children=[
          id_to_entity("Name", name),
          Entity.create(
            ~range=N.get_range(alias),
            ~attributes=[("value", fst(alias))],
            "Alias",
          ),
        ],
        "Import",
        import,
      )

    | (name, None) =>
      untyped_node_to_entity(
        ~children=[id_to_entity("Name", name)],
        "Import",
        import,
      )
    };

  let mod_stmt_to_entity = mod_stmt =>
    switch (fst(mod_stmt)) {
    | StandardImport(imports) =>
      untyped_node_to_entity(
        ~children=imports |> List.map(standard_import_to_entity),
        "StandardImport",
        mod_stmt,
      )

    | Import(namespace, imports) =>
      untyped_node_to_entity(
        ~attributes=[("namespace", Namespace.to_string(namespace))],
        ~children=imports |> List.map(import_to_entity),
        "Import",
        mod_stmt,
      )

    | Declaration(name, decl) =>
      untyped_node_to_entity(
        ~children=[export_to_entity(name), decl_to_entity(decl)],
        "Declaration",
        mod_stmt,
      )
    };

  let to_entity = program =>
    Entity.create(~children=program |> List.map(mod_stmt_to_entity), "AST");

  let pp: Fmt.t(program_t) = ppf => to_entity % Entity.pp(ppf);
};
