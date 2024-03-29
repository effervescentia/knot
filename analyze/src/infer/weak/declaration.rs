use super::{
    data::{Data, Inference, Weak},
    ToWeak,
};
use lang::{
    ast,
    types::{Enumerated, Kind, Type},
    NodeId,
};

impl ToWeak for ast::Parameter<String, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Value,
            match self {
                Self {
                    value_type: Some(x),
                    ..
                } => Data::InheritKind(*x, Kind::Type),

                Self {
                    default_value: Some(x),
                    ..
                } => Data::Inherit(*x),

                Self { .. } => Data::Infer(Inference::Parameter),
            },
        )
    }
}

impl ToWeak for ast::TypeExpression<NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Type,
            match self {
                Self::Primitive(x) => Data::Local(match x {
                    ast::TypePrimitive::Nil => Type::Nil,
                    ast::TypePrimitive::Boolean => Type::Boolean,
                    ast::TypePrimitive::Integer => Type::Integer,
                    ast::TypePrimitive::Float => Type::Float,
                    ast::TypePrimitive::String => Type::String,
                    ast::TypePrimitive::Style => Type::Style,
                    ast::TypePrimitive::Element => Type::Element,
                }),

                Self::Group(id) => Data::Inherit(**id),

                Self::Identifier(x) => Data::Infer(Inference::Reference(x.clone())),

                Self::PropertyAccess(x, property) => {
                    Data::Infer(Inference::Property(**x, property.clone()))
                }

                Self::Function(params, x) => Data::Local(Type::Function(params.clone(), **x)),
            },
        )
    }
}

impl ToWeak for ast::Declaration<String, NodeId, NodeId, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::TypeAlias { value, .. } => (Kind::Type, Data::Inherit(*value)),

            Self::Enumerated { variants, .. } => (
                Kind::Mixed,
                Data::Local(Type::Enumerated(Enumerated::Declaration(variants.clone()))),
            ),

            Self::Constant {
                value_type, value, ..
            } => (
                Kind::Value,
                value_type
                    .map(|x| Data::InheritKind(x, Kind::Type))
                    .unwrap_or(Data::Inherit(*value)),
            ),

            Self::Function {
                parameters,
                body_type,
                body,
                ..
            } => (
                Kind::Value,
                Data::Local(Type::Function(
                    parameters.clone(),
                    body_type.unwrap_or(*body),
                )),
            ),

            Self::View { parameters, .. } => {
                (Kind::Value, Data::Local(Type::View(parameters.clone())))
            }

            Self::Module { value, .. } => (Kind::Mixed, Data::Inherit(*value)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{Data, Inference, ToWeak};
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{Enumerated, Kind, Type},
        NodeId,
    };

    #[test]
    fn parameter_with_type() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(1)), None).to_weak(),
            (Kind::Value, Data::InheritKind(NodeId(1), Kind::Type))
        );
    }

    #[test]
    fn parameter_with_default() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, Some(NodeId(1))).to_weak(),
            (Kind::Value, Data::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn parameter_inference() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, None).to_weak(),
            (Kind::Value, Data::Infer(Inference::Parameter))
        );
    }

    #[test]
    fn type_expression_primitive() {
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil).to_weak(),
            (Kind::Type, Data::Local(Type::Nil))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean).to_weak(),
            (Kind::Type, Data::Local(Type::Boolean))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer).to_weak(),
            (Kind::Type, Data::Local(Type::Integer))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float).to_weak(),
            (Kind::Type, Data::Local(Type::Float))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::String).to_weak(),
            (Kind::Type, Data::Local(Type::String))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style).to_weak(),
            (Kind::Type, Data::Local(Type::Style))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element).to_weak(),
            (Kind::Type, Data::Local(Type::Element))
        );
    }

    #[test]
    fn type_expression_group() {
        assert_eq!(
            ast::TypeExpression::Group(Box::new(NodeId(1))).to_weak(),
            (Kind::Type, Data::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn type_expression_identifier() {
        assert_eq!(
            ast::TypeExpression::Identifier(str!("foo")).to_weak(),
            (Kind::Type, Data::Infer(Inference::Reference(str!("foo"))))
        );
    }

    #[test]
    fn type_expression_property_access() {
        assert_eq!(
            ast::TypeExpression::PropertyAccess(Box::new(NodeId(1)), str!("foo")).to_weak(),
            (
                Kind::Type,
                Data::Infer(Inference::Property(NodeId(1), str!("foo")))
            )
        );
    }

    #[test]
    fn type_expression_function() {
        assert_eq!(
            ast::TypeExpression::Function(vec![NodeId(1), NodeId(2)], Box::new(NodeId(3)))
                .to_weak(),
            (
                Kind::Type,
                Data::Local(Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
            )
        );
    }

    #[test]
    fn declaration_type_alias() {
        assert_eq!(
            ast::Declaration::type_alias(ast::Storage::public(str!("Foo")), NodeId(1)).to_weak(),
            (Kind::Type, Data::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn declaration_constant_explicit() {
        assert_eq!(
            ast::Declaration::constant(
                ast::Storage::public(str!("FOO")),
                Some(NodeId(1)),
                NodeId(2)
            )
            .to_weak(),
            (Kind::Value, Data::InheritKind(NodeId(1), Kind::Type))
        );
    }

    #[test]
    fn declaration_constant_implicit() {
        assert_eq!(
            ast::Declaration::constant(ast::Storage::public(str!("FOO")), None, NodeId(1))
                .to_weak(),
            (Kind::Value, Data::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn declaration_enumerated() {
        assert_eq!(
            ast::Declaration::enumerated(
                ast::Storage::public(str!("Foo")),
                vec![
                    (str!("Bar"), vec![]),
                    (str!("Fizz"), vec![NodeId(1), NodeId(2)])
                ]
            )
            .to_weak(),
            (
                Kind::Mixed,
                Data::Local(Type::Enumerated(Enumerated::Declaration(vec![
                    (str!("Bar"), vec![]),
                    (str!("Fizz"), vec![NodeId(1), NodeId(2)])
                ])))
            )
        );
    }

    #[test]
    fn declaration_function_explicit() {
        assert_eq!(
            ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![NodeId(1), NodeId(2)],
                Some(NodeId(3)),
                NodeId(4)
            )
            .to_weak(),
            (
                Kind::Value,
                Data::Local(Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
            )
        );
    }

    #[test]
    fn declaration_function_implicit() {
        assert_eq!(
            ast::Declaration::function(
                ast::Storage::public(str!("foo")),
                vec![NodeId(1), NodeId(2)],
                None,
                NodeId(3)
            )
            .to_weak(),
            (
                Kind::Value,
                Data::Local(Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
            )
        );
    }

    #[test]
    fn declaration_view() {
        assert_eq!(
            ast::Declaration::view(
                ast::Storage::public(str!("Foo")),
                vec![NodeId(1), NodeId(2)],
                NodeId(3)
            )
            .to_weak(),
            (
                Kind::Value,
                Data::Local(Type::View(vec![NodeId(1), NodeId(2)]))
            )
        );
    }

    #[test]
    fn declaration_module() {
        assert_eq!(
            ast::Declaration::module(ast::Storage::public(str!("foo")), NodeId(1)).to_weak(),
            (Kind::Mixed, Data::Inherit(NodeId(1)))
        );
    }
}
