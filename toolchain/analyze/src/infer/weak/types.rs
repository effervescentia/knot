use super::{
    data::{Inference, Type, Weak},
    to_weak::ToWeak,
};
use lang::{
    ast::{self, ObjectTypeExpressionEntry},
    types::{self, Kind, ObjectTypeEntry},
    NodeId,
};

fn object_to_weak(entries: &[ObjectTypeExpressionEntry<String, NodeId>]) -> Type {
    let result = entries
        .iter()
        .map(|entry| match entry {
            ObjectTypeExpressionEntry::Required(name, x) => {
                Some(ObjectTypeEntry::Required(name.clone(), *x))
            }

            ObjectTypeExpressionEntry::Optional(name, x) => {
                Some(ObjectTypeEntry::Optional(name.clone(), *x))
            }

            ObjectTypeExpressionEntry::Spread(_) => None,
        })
        .collect::<Option<Vec<_>>>();

    if let Some(entries) = result {
        Type::Value(types::Type::Object(entries))
    } else {
        Type::Infer(Inference::ObjectType(entries.to_vec()))
    }
}

impl ToWeak for ast::TypeExpression<String, NodeId> {
    fn to_weak(&self) -> Weak {
        (
            Kind::Type,
            match self {
                Self::Primitive(x) => Type::Value(match x {
                    ast::TypePrimitive::Nil => types::Type::Nil,
                    ast::TypePrimitive::Boolean => types::Type::Boolean,
                    ast::TypePrimitive::Integer => types::Type::Integer,
                    ast::TypePrimitive::Float => types::Type::Float,
                    ast::TypePrimitive::String => types::Type::String,
                    ast::TypePrimitive::Style => types::Type::Style,
                    ast::TypePrimitive::Element => types::Type::Element,
                }),

                Self::Group(id) => Type::Inherit(**id),

                Self::Identifier(x) => Type::Infer(Inference::Reference(x.clone(), None)),

                Self::PropertyAccess(x, property) => {
                    Type::Infer(Inference::Property(**x, property.clone()))
                }

                Self::Function(params, x) => {
                    Type::Value(types::Type::Function(params.clone(), **x))
                }

                Self::Object(entries) => object_to_weak(entries),
            },
        )
    }
}

impl ToWeak for ast::TypeDeclaration<String, NodeId, NodeId> {
    fn to_weak(&self) -> Weak {
        match self {
            Self::TypeAlias { value, .. } => (Kind::Type, Type::Inherit(*value)),

            Self::Enumerated { variants, .. } => (
                Kind::Type,
                Type::Value(types::Type::Enumerated(types::Enumerated::Declaration(
                    variants.clone(),
                ))),
            ),

            Self::View { attributes, .. } => {
                (Kind::Value, Type::Infer(Inference::ViewType(*attributes)))
            }

            Self::Function {
                parameters, result, ..
            } => (
                Kind::Value,
                Type::Value(types::Type::Function(parameters.clone(), *result)),
            ),

            Self::Module { module, .. } => (Kind::Value, Type::Inherit(*module)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{Inference, ToWeak, Type};
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{self, Enumerated, Kind},
        NodeId,
    };

    #[test]
    fn parameter_with_type() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), Some(NodeId(1)), None).to_weak(),
            (Kind::Value, Type::InheritKind(NodeId(1), Kind::Type))
        );
    }

    #[test]
    fn parameter_with_default() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, Some(NodeId(1))).to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn parameter_inference() {
        assert_eq!(
            ast::Parameter::new(str!("foo"), None, None).to_weak(),
            (Kind::Value, Type::Infer(Inference::Parameter(str!("foo"))))
        );
    }

    #[test]
    fn type_expression_primitive() {
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil).to_weak(),
            (Kind::Type, Type::Value(types::Type::Nil))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean).to_weak(),
            (Kind::Type, Type::Value(types::Type::Boolean))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer).to_weak(),
            (Kind::Type, Type::Value(types::Type::Integer))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float).to_weak(),
            (Kind::Type, Type::Value(types::Type::Float))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::String).to_weak(),
            (Kind::Type, Type::Value(types::Type::String))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style).to_weak(),
            (Kind::Type, Type::Value(types::Type::Style))
        );
        assert_eq!(
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element).to_weak(),
            (Kind::Type, Type::Value(types::Type::Element))
        );
    }

    #[test]
    fn type_expression_group() {
        assert_eq!(
            ast::TypeExpression::Group(Box::new(NodeId(1))).to_weak(),
            (Kind::Type, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn type_expression_identifier() {
        assert_eq!(
            ast::TypeExpression::Identifier(str!("foo")).to_weak(),
            (
                Kind::Type,
                Type::Infer(Inference::Reference(str!("foo"), None))
            )
        );
    }

    #[test]
    fn type_expression_property_access() {
        assert_eq!(
            ast::TypeExpression::PropertyAccess(Box::new(NodeId(1)), str!("foo")).to_weak(),
            (
                Kind::Type,
                Type::Infer(Inference::Property(NodeId(1), str!("foo")))
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
                Type::Value(types::Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
            )
        );
    }

    #[test]
    fn type_expression_object_known_properties() {
        assert_eq!(
            ast::TypeExpression::Object(vec![
                ast::ObjectTypeExpressionEntry::Required(str!("foo"), NodeId(1)),
                ast::ObjectTypeExpressionEntry::Optional(str!("bar"), NodeId(2)),
            ])
            .to_weak(),
            (
                Kind::Type,
                Type::Value(types::Type::Object(vec![
                    types::ObjectTypeEntry::Required(str!("foo"), NodeId(1)),
                    types::ObjectTypeEntry::Optional(str!("bar"), NodeId(2)),
                ]))
            )
        );
    }

    #[test]
    fn type_expression_object_inferred_properties() {
        assert_eq!(
            ast::TypeExpression::Object(vec![
                ast::ObjectTypeExpressionEntry::Required(str!("foo"), NodeId(1)),
                ast::ObjectTypeExpressionEntry::Optional(str!("bar"), NodeId(2)),
                ast::ObjectTypeExpressionEntry::Spread(NodeId(3)),
            ])
            .to_weak(),
            (
                Kind::Type,
                Type::Infer(Inference::ObjectType(vec![
                    ast::ObjectTypeExpressionEntry::Required(str!("foo"), NodeId(1)),
                    ast::ObjectTypeExpressionEntry::Optional(str!("bar"), NodeId(2)),
                    ast::ObjectTypeExpressionEntry::Spread(NodeId(3)),
                ]))
            )
        );
    }

    #[test]
    fn declaration_type_alias() {
        assert_eq!(
            ast::Declaration::type_alias(ast::Storage::public(str!("Foo")), NodeId(1)).to_weak(),
            (Kind::Type, Type::Inherit(NodeId(1)))
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
            (Kind::Value, Type::InheritKind(NodeId(1), Kind::Type))
        );
    }

    #[test]
    fn declaration_constant_implicit() {
        assert_eq!(
            ast::Declaration::constant(ast::Storage::public(str!("FOO")), None, NodeId(1))
                .to_weak(),
            (Kind::Value, Type::Inherit(NodeId(1)))
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
                Type::Value(types::Type::Enumerated(Enumerated::Declaration(vec![
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
                Type::Value(types::Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
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
                Type::Value(types::Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
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
                Type::Infer(Inference::View(vec![NodeId(1), NodeId(2)]))
            )
        );
    }

    #[test]
    fn declaration_module() {
        assert_eq!(
            ast::Declaration::module(ast::Storage::public(str!("foo")), NodeId(1)).to_weak(),
            (Kind::Mixed, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn type_declaration_type_alias() {
        assert_eq!(
            ast::TypeDeclaration::type_alias(str!("Foo"), NodeId(1)).to_weak(),
            (Kind::Type, Type::Inherit(NodeId(1)))
        );
    }

    #[test]
    fn type_declaration_view() {
        assert_eq!(
            ast::TypeDeclaration::view(str!("Foo"), NodeId(1)).to_weak(),
            (Kind::Value, Type::Infer(Inference::ViewType(NodeId(1))))
        );
    }

    #[test]
    fn type_declaration_function() {
        assert_eq!(
            ast::TypeDeclaration::function(str!("foo"), vec![NodeId(1), NodeId(2)], NodeId(3))
                .to_weak(),
            (
                Kind::Value,
                Type::Value(types::Type::Function(vec![NodeId(1), NodeId(2)], NodeId(3)))
            )
        );
    }
}
