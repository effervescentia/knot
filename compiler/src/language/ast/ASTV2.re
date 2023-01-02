open Knot.Kore;

type raw_t('a) = Node.t('a, unit);
type identifier_t = raw_t(string);

module Primitive = {
  type t =
    | Nil
    | Boolean(bool)
    | Integer(Int64.t)
    | Float(float, int)
    | String(string);
};

module TypeExpression = {
  module ObjectEntry = {
    type t('expr) =
      | Required(identifier_t, 'expr)
      | Optional(identifier_t, 'expr)
      | Spread('expr);

    type node_t('expr) = raw_t(t('expr));
  };

  type t =
    | Nil
    | Boolean
    | Integer
    | Float
    | String
    | Style
    | Element
    | Identifier(string)
    | Group(node_t)
    | List(node_t)
    | Object(list(ObjectEntry.node_t(node_t)))
    | Function(list(node_t), node_t)
    | DotAccess(node_t, identifier_t)
    | View(node_t, node_t)

  and node_t = raw_t(t);
};

module Statement = {
  type t('expr) =
    | Variable(identifier_t, 'expr)
    | Effect('expr);

  type node_t('expr, 'typ) = Node.t(t('expr), 'typ);
};

module KSX = {
  module Child = {
    type t('expr, 'ksx) =
      | Text(string)
      | Inline('expr)
      | KSX('ksx);

    type node_t('expr, 'ksx) = raw_t(t('expr, 'ksx));
  };

  type tag_t =
    | View
    | Element;

  type t('expr, 'typ) =
    | Tag(
        tag_t,
        Node.t(string, 'typ),
        list('expr),
        list(raw_t((identifier_t, option('expr)))),
        list(Child.node_t('expr, t('expr, 'typ))),
      )
    | Fragment(Child.node_t('expr, t('expr, 'typ)))

  and node_t('expr, 'typ) = raw_t(t('expr, 'typ));
};

module Expression = {
  type t('typ) =
    | Primitive(Primitive.t)
    | Identifier(string)
    | Style(list(raw_t((identifier_t, node_t('typ)))))
    | Group(node_t('typ))
    | Closure(list(Statement.node_t(t('typ), 'typ)))
    | DotAccess(node_t('typ), identifier_t)
    | UnaryOperation(Operator.Unary.t, node_t('typ))
    | BinaryOperation(Operator.Binary.t, node_t('typ), node_t('typ))
    | BindStyle(KSX.tag_t, node_t('typ), node_t('typ))
    | FunctionCall(node_t('typ), list(node_t('typ)))
    | KSX(KSX.node_t(t('typ), 'typ))

  and node_t('typ) = Node.t(t('typ), 'typ);
};

module Declaration = {
  module Variant = {
    type t('typ) = (
      identifier_t,
      list(Node.t(TypeExpression.node_t, 'typ)),
    );

    type node_t('typ) = Node.t(t('typ), 'typ);
  };

  module Argument = {
    type t('typ) = (
      identifier_t,
      option(Node.t(TypeExpression.node_t, 'typ)),
      option(Expression.node_t('typ)),
    );

    type node_t('typ) = Node.t(t('typ), 'typ);
  };

  type t('typ) =
    | Enumerated(list(Variant.node_t('typ)))
    | Constant(Expression.node_t('typ))
    | Function(list(Argument.node_t('typ)), Expression.node_t('typ))
    | View(
        list(Argument.node_t('typ)),
        list(Expression.node_t('typ)),
        Expression.node_t('typ),
      );

  type node_t('typ) = Node.t(t('typ), 'typ);
};

module ModuleStatement = {
  module NamedImport = {
    type t = (identifier_t, option(identifier_t));

    type node_t = raw_t(t);
  };

  type export_t =
    | MainExport
    | NamedExport;

  type t('typ) =
    | Export(export_t, identifier_t, Declaration.node_t('typ))
    | StdlibImport(list(NamedImport.node_t))
    | Import(
        raw_t(Reference.Namespace.t),
        option(identifier_t),
        list(NamedImport.node_t),
      );

  type node_t('typ) = raw_t(t('typ));
};

module Program = {
  type t('typ) = list(ModuleStatement.node_t('typ));
};
